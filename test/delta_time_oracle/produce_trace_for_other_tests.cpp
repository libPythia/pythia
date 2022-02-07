#include <eta/oracle/delta_time.h>

#include "common.hpp"

auto main() -> int {
    eta_dt_oracle_init(EVENT_COUNT);
    run<false>();
    eta_dt_oracle_deinit();
}

