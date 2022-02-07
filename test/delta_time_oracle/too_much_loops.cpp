#include <eta/oracle/delta_time.h>

#include "common.hpp"

auto main() -> int {
    eta_dt_oracle_init(EVENT_COUNT);
    run<true>();
    // Copy of the code of run<true>(), excepting that the first loop should lead to bad prediction
    // do to predicting end of loop
    auto first = true;
    for (auto i = 0u; i < 100u; ++i) {
        for (auto j = 0u; j < 100u; ++j)
            if (first) {
                ac<false>();
                first = false;
            } else {
                ac<true>();
            }
        for (auto j = 0u; j < 200u; ++j)
            bc<true>();
    }
    eta_dt_oracle_deinit();
}

