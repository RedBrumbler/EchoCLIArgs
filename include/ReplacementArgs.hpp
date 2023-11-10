#pragma once

#include <string>

namespace EchoCLIArgs {
struct ReplacementArgs {
  static std::string read_args();
  static void install_hook();

  static bool apply_args(char* game_args);
};
}  // namespace EchoCLIArgs
