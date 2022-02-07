
eta_factorizer_exaustive_test = executable('exaustive_test',
                                  'exaustive_test.cpp',
                                  dependencies:[eta_factorizer_dep, thread_dep, utils_dep])

test('eta_reduce', eta_factorizer_exaustive_test, args:['-s10', '-a5'], is_parallel : false)

#----------------------------------------------------------------

value_oracle_tests=[
  'predict_begining_of_trace',
  'one_value_to_predict_equal_to_default',
  'one_value_to_predict_different_from_default',
  'two_values_to_predict_with_separators',
  'two_values_to_predict_without_separators',
  'predict_at_end_of_simple_loop',
  'predict_at_end_of_complex_loop',
  'predict_end_of_loop',
  'predict_at_end_of_loop_of_prediction',
  'predict_at_end_of_loop_with_prediction',
  'predict_at_end_of_nested_loop_with_prediction',
  'no_prediction_to_do',
  'predict_after_failure_simple',
  'predict_after_failure_by_probability_1',
  'predict_after_failure_by_probability_2',
]

foreach t : value_oracle_tests
  test(t,
       executable('value_oracle_' + t, 'value_oracle/' + t + '.cpp',
                  dependencies:[eta_oracle_dep]),
       suite:'value_oracle')
endforeach

test('produce trace',
     executable('value_oracle_procuce_trace', 'value_oracle/produce_trace.cpp', dependencies:[eta_oracle_dep]),
     suite:'value_oracle',
     is_parallel: false,
     priority: 0) # Must be executed BEFORE 'consume trace'

test('consume_trace',
     executable('value_oracle_consume_trace', 'value_oracle/consume_trace.cpp', dependencies:[eta_oracle_dep]),
     suite:'value_oracle',
     is_parallel: false,
     priority: -1, # Must be executed AFTER 'produce trace'
     env:['ETA_MODE=PREDICT'])

#----------------------------------------------------------------

test('basic_test',
     executable('dt_oracle_basic',
                'delta_time_oracle/basic.cpp',
                dependencies:[eta_oracle_dep]),
     suite:'delta_time_oracle',
     priority: 10, # Must be executed first
     env:['ETA_MODE=RECORD'])

test('produce trace',
     executable('dt_oracle_procuce_trace',
                'delta_time_oracle/produce_trace.cpp',
                dependencies:[eta_oracle_dep]),
     suite:'delta_time_oracle',
     is_parallel: false,
     priority: 5, # Must be executed BEFORE 'consume trace'
     env:['ETA_MODE=RECORD'])

test('consume_trace',
     executable('dt_oracle_consume_trace',
                'delta_time_oracle/consume_trace.cpp',
                dependencies:[eta_oracle_dep]),
     suite:'delta_time_oracle',
     is_parallel: false,
     priority: 4, # Must be executed AFTER 'produce trace'
     env:['ETA_MODE=PREDICT'])

test('produce_trace_for_other_tests',
    executable('dt_oracle_produce_trace_for_tests',
               'delta_time_oracle/produce_trace_for_other_tests.cpp',
               dependencies:[eta_oracle_dep]),
    suite:'delta_time_oracle',
    is_parallel: false,
    priority: 3, # Must be executed after produce_trace and consume_trace and before
                 # all the other tests
    env:['ETA_MODE=RECORD'])

foreach t : ['branching', 'variation', 'too_much_loops']
  test(t,
       executable('dt_oracle_' + t, 'delta_time_oracle/' + t + '.cpp',
                  dependencies:[eta_oracle_dep]),
       suite:'delta_time_oracle',
       priority: 0, # Must be executed last
       env:['ETA_MODE=PREDICT'])
endforeach

#----------------------------------------------------------------

eta_factorizer_bench = executable('bench',
                             'bench.cpp',
                             dependencies:[eta_factorizer_dep, utils_dep])

