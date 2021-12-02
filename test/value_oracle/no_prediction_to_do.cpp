#include <eta/oracle/predict_value.h>

#include "check.hpp"

auto main() -> int {
    // create_trace
    eta_init_value_oracle_recording(2);
    eta_append_event(0);
    eta_append_event(1);
    eta_append_event(0);
    eta_append_event(1);

    // predict
    eta_switch_value_oracle_to_prediction();
    eta_append_event(0);
    eta_append_event(1);
    check(100, eta_predict_value(100));

    eta_deinit_value_oracle();
    return 0;
}

