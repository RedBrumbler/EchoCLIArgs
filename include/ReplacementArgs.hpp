#pragma once

#include <string>

namespace EchoCLIArgs {
struct ReplacementArgs {
  static std::string read_args();
  static void install_hook();
};
}  // namespace EchoCLIArgs
