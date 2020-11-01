#include "spoor/instrumentation/config/config.h"

#include "util/env.h"

namespace spoor::instrumentation::config {

auto Config::FromEnv(const util::env::GetEnv& get_env) -> Config {
  (void)get_env;  // TODO
  return {};
}

}  // namespace spoor::instrumentation::config
