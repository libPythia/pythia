#include <eta/oracle/delta_time.h>

#include "common.hpp"

auto main() -> int {
    eta_dt_oracle_init(EVENT_COUNT);
    run<true>();
    eta_dt_oracle_deinit();
}

