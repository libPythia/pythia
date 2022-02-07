#include <eta/oracle/delta_time.h>

#include "common.hpp"

eta_dt_oracle_prediction prediction;

auto main() -> int {
    eta_dt_oracle_init(EVENT_COUNT);
    // Start with destroying knowledge about the starting program with an unexpected event
    eta_dt_oracle_add_event(UNKNOWN_EVENT);
    // Check no prediction is available now
    eta_dt_oracle_get_prediction(&prediction);
    check(prediction.type < 0);
    // Take feet with in C zone and check we get a mean of ac and bc pattern context duration
    eta_dt_oracle_add_event(START_C_ZONE);
    make_prediction_and_check<true>((5e-4 + 2 * 2e-5) / 3., (5e-3 + 2 * 2e-4) / 3, STOP_C_ZONE);

    eta_dt_oracle_deinit();
}

