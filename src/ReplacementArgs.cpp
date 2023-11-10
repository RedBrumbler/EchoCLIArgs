#include "ReplacementArgs.hpp"

#include <asm-generic/mman-common.h>
#include <dlfcn.h>
#include <filesystem>
#include <fstream>

#include "Util.hpp"
#include "log.h"
#include "yodel/shared/modloader.h"

#include "flamingo/shared/trampoline-allocator.hpp"
#include "flamingo/shared/trampoline.hpp"

namespace EchoCLIArgs {
uint32_t* get_target_address() {
  auto setcommandlineargs =
      dlsym(modloader_r15_handle, "_ZN10NRadEngine8CSysJava7GetArgsERNS_13CFixedStringTILy256EEE");
  if (!setcommandlineargs) {
    LOG_ERROR(
        "Could not find symbol "
        "_ZN10NRadEngine8CSysJava7GetArgsERNS_13CFixedStringTILy256EEE "
        "in libr15.so! can't provide hook address");
  }

  return static_cast<uint32_t*>(setcommandlineargs);
}
void ReplacementArgs::install_hook() {
  // find target symbol
  auto target = get_target_address();
  if (!target) return;

  Util::protect(target, PROT_READ | PROT_WRITE | PROT_EXEC);

  // install a hook
  static auto trampoline = flamingo::TrampolineAllocator::Allocate(64);
  trampoline.WriteHookFixups(target);
  trampoline.WriteCallback(&target[4]);
  trampoline.Finish();

  static auto args_hook = [](void* self, char* args) noexcept {
    // call into apply args to override args. if this is not successful just run
    // orig
    if (!apply_args(args)) {
      LOG_WARN("custom arguments not found, so we have not set them");
      reinterpret_cast<void (*)(void*, char*)>(trampoline.address.data())(self, args);
    }
  };

  std::size_t trampoline_size = 64;
  std::size_t num_insts = 8;
  auto target_hook = flamingo::Trampoline(target, num_insts, trampoline_size);
  target_hook.WriteCallback(reinterpret_cast<uint32_t*>(+args_hook));
  target_hook.Finish();

  Util::protect(target, PROT_READ | PROT_EXEC);
}

bool ReplacementArgs::apply_args(char* game_args) {
  // read the args, store their result somewhere and apply that value
  auto args = read_args();
  if (args.empty()) return false;

  LOG_DEBUG("Found replacement args: {}", args);

  // the game_args array is max 0x100 bytes (256), taking the terminating byte
  // into account that is 255 chars allowed
  auto len = std::min(args.size(), 255ul);
  if (len != args.size()) {
    LOG_WARN(
        "Args were of length {}, but the maximum allowed is 255. args "
        "will be forcibly truncated!",
        args.size());
  }

  std::memcpy(reinterpret_cast<void*>(game_args), reinterpret_cast<void const*>(args.c_str()), len);
  return true;
}

std::string ReplacementArgs::read_args() {
  std::filesystem::path filepath{ fmt::format("/sdcard/ModData/{}/Mods/echo-cli-args/args.txt",
                                              modloader::get_application_id()) };

  // find args file on disk
  if (!std::filesystem::exists(filepath)) return "";

  // read contents & return
  auto file = std::ifstream(filepath, std::ios::in | std::ios::ate);
  auto len = file.tellg();
  file.seekg(0);
  std::string data;
  data.resize(len);
  file.read(data.data(), len);

  return data;
}
}  // namespace EchoCLIArgs
