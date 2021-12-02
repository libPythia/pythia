#include <eta/oracle/predict_value.h>

#include "check.hpp"

auto main() -> int {
    // create_trace
    eta_init_value_oracle_recording(3);
    eta_append_event(0);
    check(100, eta_predict_value(100));
    eta_correct_value_prediction(25);
    eta_append_event(1);
    check(50, eta_predict_value(50));
    eta_correct_value_prediction(75);
    eta_append_event(0);
    check(100, eta_predict_value(100));
    eta_correct_value_prediction(25);

    // predict
    eta_switch_value_oracle_to_prediction();
    eta_append_event(1);
    check(75, eta_predict_value(150));
    check(25, eta_predict_value(30));

    eta_deinit_value_oracle();
    return 0;
}

