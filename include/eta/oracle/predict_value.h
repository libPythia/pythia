#ifndef ETA_REDUCTION_H
#define ETA_REDUCTION_H

#ifdef __cplusplus
extern "C" {
#endif

void eta_init_value_oracle_recording(unsigned int event_type_count);
void eta_init_value_oracle(unsigned int event_type_count);
// void eta_init_value_oracle_predicting(char const *);
// void eta_save_recorded_data(char const *);
void eta_deinit_value_oracle();
void eta_switch_value_oracle_to_prediction();

// request a prediction about the best thread number to use for next bloc of code
// if no prediction is available, return omp_get_max_threads()
unsigned char eta_predict_value(unsigned char default_value);

// Gives fedback over the last prediction for next run
void eta_correct_value_prediction(unsigned char value);

// Inform an oracle of a new occurence of a given event
void eta_append_event(unsigned int event_type);

#ifdef __cplusplus
}
#endif

#endif  // ETA_REDUCTION_H

