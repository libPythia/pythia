#include <eta/oracle/predict_value.h>

#include "check.hpp"

auto main() -> int {
    // create_trace
    eta_init_value_oracle_recording(2);
    eta_append_event(0);
    for (auto i = 0; i < 10; ++i) {
        check(100, eta_predict_value(100));
        eta_correct_value_prediction(50);
    }
    check(25, eta_predict_value(25));
    eta_correct_value_prediction(30);
    eta_append_event(1);

    // predict
    eta_switch_value_oracle_to_prediction();
    eta_append_event(0);
    for (auto i = 0; i < 10; ++i) {
        check(50, eta_predict_value(100));
        eta_correct_value_prediction(50);
    }
    check(30, eta_predict_value(25));
    eta_correct_value_prediction(30);
    eta_append_event(1);

    eta_deinit_value_oracle();
    return 0;
}

