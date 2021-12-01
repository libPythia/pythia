#include <eta/oracle/predict_value.h>

#include "check.hpp"

auto main() -> int {
    // create_trace
    eta_init_value_oracle_recording(3);
    for (auto i = 0; i < 10; ++i) {
        eta_append_event(0);
        check(30, eta_predict_value(30));
        eta_correct_value_prediction(15);
        eta_append_event(1);
    }
    eta_append_event(0);
    check(100, eta_predict_value(100));
    eta_correct_value_prediction(50);
    eta_append_event(2);

    // predict
    eta_switch_value_oracle_to_prediction();
    for (auto i = 0; i < 10; ++i) {
        eta_append_event(0);
        check(15, eta_predict_value(30));
        eta_correct_value_prediction(15);
        eta_append_event(1);
    }
    eta_append_event(0);
    check(50, eta_predict_value(100));
    eta_correct_value_prediction(50);
    eta_append_event(2);

    eta_deinit_value_oracle();
    return 0;
}

