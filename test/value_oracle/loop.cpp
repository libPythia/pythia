#include <eta/oracle/predict_value.h>

#include "check.hpp"

auto main() -> int {
    // create_trace
    eta_init_value_oracle_recording(1);
    eta_append_event(0);
    for (auto i = 0; i < 100; ++i) {
        check(100, eta_predict_value(100));
        eta_correct_value_prediction(i);
    }

    // predict
    eta_switch_value_oracle_to_prediction();
    eta_append_event(0);
    for (auto i = 0; i < 100; ++i) {
        check(i, eta_predict_value(100));
        eta_correct_value_prediction(i);
    }

    eta_deinit_value_oracle();
    return 0;
}

