#include <eta/oracle/predict_value.h>

#include "check.hpp"

static auto a = []() { eta_append_event(0); };
static auto bc = []() {
    check(2, eta_predict_value(6));
    eta_correct_value_prediction(7);
};
static auto bd = []() {
    check(3, eta_predict_value(8));
    eta_correct_value_prediction(9);
};
static auto ef = []() {
    check(5, eta_predict_value(10));
    eta_correct_value_prediction(11);
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
