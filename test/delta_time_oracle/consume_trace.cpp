#include <eta/oracle/delta_time.h>

#include <chrono>
#include <thread>

#include "check.hpp"

static auto sleep_ms(int ms) { std::this_thread::sleep_for(std::chrono::milliseconds(ms)); }

enum Events {
    START_A_ZONE,
    START_B_ZONE,
    STOP_ZONE,
    EVENT_COUNT,
};

template <bool is_predicting>
static auto run_zone(Events zone, double duration, double min, double max) {
    eta_dt_oracle_add_event(zone);
    if constexpr (is_predicting) {
        eta_dt_oracle_prediction prediction;
        eta_dt_oracle_get_prediction(&prediction);
        check(prediction.dt > min);
        check(prediction.dt < max);
        check(prediction.type == STOP_ZONE);
    }
    sleep_ms(duration);
    eta_dt_oracle_add_event(STOP_ZONE);
}

template <bool is_predicting> static auto test() -> void {
    // TODO
    for (auto i = 0; i < 100; ++i) {
        run_zone<is_predicting>(START_A_ZONE, 2, 1e-3, 3e-3);
        run_zone<is_predicting>(START_B_ZONE, 20, 18e-3, 22e-3);
    }
}

auto main() -> int {
    eta_dt_oracle_init(EVENT_COUNT);
    test<true>();
    eta_dt_oracle_deinit();
}

