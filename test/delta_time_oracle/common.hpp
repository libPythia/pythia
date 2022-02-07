#pragma once

#include <eta/oracle/delta_time.h>

#include <chrono>
#include <thread>

#include "check.hpp"

template <bool is_predicting> static auto sleep_us(int us) {
    if constexpr (is_predicting == false) {
        std::this_thread::sleep_for(std::chrono::microseconds(us));
    }
}

enum Events {
    START_A_ZONE,
    STOP_A_ZONE,
    START_B_ZONE,
    STOP_B_ZONE,
    START_C_ZONE,
    STOP_C_ZONE,
    UNKNOWN_EVENT,
    EVENT_COUNT,
};

template <bool is_predicting> auto make_prediction_and_check(double min, double max, Events event) {
    if constexpr (is_predicting) {
        eta_dt_oracle_prediction prediction;
        eta_dt_oracle_get_prediction(&prediction);
        check(prediction.type == event);
        check(prediction.dt > min);
        check(prediction.dt < max);
    }
}

template <bool is_predicting> static auto ac() {
    eta_dt_oracle_add_event(START_A_ZONE);
    make_prediction_and_check<is_predicting>(3e-5, 3e-4, STOP_A_ZONE);
    sleep_us<is_predicting>(30);
    eta_dt_oracle_add_event(STOP_A_ZONE);
    eta_dt_oracle_add_event(START_C_ZONE);
    make_prediction_and_check<is_predicting>(5e-4, 5e-3, STOP_C_ZONE);
    sleep_us<is_predicting>(500);
    eta_dt_oracle_add_event(STOP_C_ZONE);
}

template <bool is_predicting> static auto bc() {
    eta_dt_oracle_add_event(START_B_ZONE);
    make_prediction_and_check<is_predicting>(5e-5, 5e-4, STOP_B_ZONE);
    sleep_us<is_predicting>(50);
    eta_dt_oracle_add_event(STOP_B_ZONE);
    eta_dt_oracle_add_event(START_C_ZONE);
    make_prediction_and_check<is_predicting>(2e-5, 2e-4, STOP_C_ZONE);
    sleep_us<is_predicting>(20);
    eta_dt_oracle_add_event(STOP_C_ZONE);
}

template <bool is_predicting> static auto run() {
    for (auto i = 0u; i < 100u; ++i) {
        for (auto j = 0u; j < 100u; ++j)
            ac<is_predicting>();
        for (auto j = 0u; j < 200u; ++j)
            bc<is_predicting>();
    }
}

