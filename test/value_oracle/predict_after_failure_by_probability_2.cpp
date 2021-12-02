#include <eta/oracle/predict_value.h>

#include "check.hpp"

auto main() -> int {
    // create_trace
    eta_init_value_oracle_recording(9);

    eta_append_event(0);
    check(100, eta_predict_value(100));
    eta_correct_value_prediction(25);

    eta_append_event(1);
    check(100, eta_predict_value(100));
    eta_correct_value_prediction(5);

    eta_append_event(2);
    check(100, eta_predict_value(100));
    eta_correct_value_prediction(25);

    eta_append_event(1);
    check(100, eta_predict_value(100));
    eta_correct_value_prediction(25);

    eta_append_event(3);
    check(100, eta_predict_value(100));
    eta_correct_value_prediction(2);

    eta_append_event(1);
    check(100, eta_predict_value(100));
    eta_correct_value_prediction(5);

    eta_append_event(4);
    check(100, eta_predict_value(100));
    eta_correct_value_prediction(25);

    // predict
    eta_switch_value_oracle_to_prediction();
    eta_append_event(1);
    check(5, eta_predict_value(150));  // after a 1
    check(25, eta_predict_value(30));  // from nowhere
    eta_append_event(3);
    check(2, eta_predict_value(30));  // after a 3

    eta_deinit_value_oracle();
    return 0;
}

