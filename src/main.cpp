#include "yodel/shared/loader.hpp"

#include "ReplacementArgs.hpp"
#include "log.h"

extern "C" void setup(CModInfo* info) {
  info->id = MOD_ID;
  info->version = VERSION;
  info->version_long = GIT_COMMIT;
}

extern "C" void load() {
  LOG_INFO("Providing echovr cli args with branch " GIT_BRANCH " ({}) by " GIT_USER, GIT_COMMIT);
  EchoCLIArgs::ReplacementArgs::install_hook();
}
