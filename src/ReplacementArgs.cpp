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
using namespace std::string_view_literals;

uint32_t* get_target_address() {
  static auto constexpr symbol = "_ZN10NRadEngine5Main_EPKc"sv;
  auto engine_main = dlsym(modloader_r15_handle, symbol.data());
  if (!engine_main) {
    LOG_ERROR("Could not find symbol '{}' in libr15.so! can't provide hook address", symbol);
  }

  return static_cast<uint32_t*>(engine_main);
}

char const** get_args_address() {
  static constexpr auto offset = 0x372c000;
  auto base = Util::baseAddr("libr15-original.so");
  if (!base) {
    LOG_WARN("Could not get base address for libr15-original.so, not returning valid args address!");
    return nullptr;
  }
  return (char const**)(base + offset);
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

  static auto main_hook = [](char const* cli_args) noexcept {
    // get the replacement args
    static auto replacement_args = read_args();

    // if they are not empty, apply them
    if (!replacement_args.empty()) {
      LOG_DEBUG("Replacing cli arguments with: '{}'", replacement_args);
      // get the static address of the cli args pointer in r15 and override if found
      auto args_address = get_args_address();
      if (args_address) *args_address = replacement_args.c_str();
      // override args passed
      cli_args = replacement_args.c_str();
    } else {
      LOG_WARN("custom arguments not found, so we have not set them");
    }

    // call orig
    reinterpret_cast<void (*)(char const*)>(trampoline.address.data())(cli_args);
  };

  std::size_t trampoline_size = 64;
  std::size_t num_insts = 8;
  auto target_hook = flamingo::Trampoline(target, num_insts, trampoline_size);
  target_hook.WriteCallback(reinterpret_cast<uint32_t*>(+main_hook));
  target_hook.Finish();

  Util::protect(target, PROT_READ | PROT_EXEC);
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
