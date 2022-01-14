#include <eta/oracle/predict_value.h>

#include "check.hpp"

static auto a = []() { eta_append_event(0); };
static auto bc = []() {
    check(1, eta_predict_value(1));
    eta_correct_value_prediction(2);
};
static auto bd = []() {
    check(1, eta_predict_value(1));
    eta_correct_value_prediction(3);
};
static auto ef = []() {
    check(4, eta_predict_value(4));
    eta_correct_value_prediction(5);
};

auto main() -> int {
    eta_init_value_oracle(1);
    for (auto i = 0; i < 100; ++i) {
        a();
        if (i % 2 == 0)
            bc();
        else
            bd();
    }

    for (auto i = 0; i < 1000; ++i)
        ef();

    for (auto i = 0; i < 10; ++i) {
        a();
        bd();
    }
    eta_deinit_value_oracle();
}
