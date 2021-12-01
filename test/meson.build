
eta_factorizer_exaustive_test = executable('exaustive_test',
                                  'exaustive_test.cpp',
                                  dependencies:[eta_factorizer_dep, thread_dep, utils_dep])

test('eta_reduce', eta_factorizer_exaustive_test, args:['-s10', '-a5'], is_parallel : false)


eta_factorizer_prediction_test = executable('prediction_test',
                                  'prediction_test.cpp',
                                  dependencies:[eta_factorizer_dep, thread_dep, utils_dep])

test('eta_predict', eta_factorizer_prediction_test, args:['-s9', '-a5'], is_parallel : false)

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
]

foreach t : value_oracle_tests
  test(t,
       executable('test_value_oracle_' + t, 'value_oracle/' + t + '.cpp',
                  dependencies:[eta_oracle_dep]),
       suite:'oracle')
endforeach

#----------------------------------------------------------------

eta_factorizer_bench = executable('bench',
                             'bench.cpp',
                             dependencies:[eta_factorizer_dep, utils_dep])

