#ifndef ETA_REDUCTION_H
#define ETA_REDUCTION_H

#ifdef __cplusplus
extern "C" {
#endif

int eta_dt_oracle_is_active();
int eta_dt_oracle_is_prediction_enabled();

void eta_dt_oracle_init(unsigned int event_type_count);
void eta_dt_oracle_switch_to_prediction();
void eta_dt_oracle_deinit();

struct eta_dt_oracle_prediction {
    double dt;
    int type;
};

void eta_dt_oracle_add_event(unsigned int);
void eta_dt_oracle_get_prediction(struct eta_dt_oracle_prediction * prediction);

#ifdef __cplusplus
}
#endif

#endif  // ETA_REDUCTION_H

