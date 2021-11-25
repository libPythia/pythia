
eta_factorizer_exaustive_test = executable('exaustive_test',
                                  'src/exaustive_test.cpp',
                                  dependencies:[eta_factorizer_dep, thread_dep, utils_dep])

test('eta_reduce', eta_factorizer_exaustive_test, args:['-s10', '-a5'], is_parallel : false)


eta_factorizer_prediction_test = executable('prediction_test',
                                  'src/prediction_test.cpp',
                                  dependencies:[eta_factorizer_dep, thread_dep, utils_dep])

test('eta_predict', eta_factorizer_prediction_test, args:['-s9', '-a5'], is_parallel : false)

#----------------------------------------------------------------

eta_factorizer_bench = executable('bench',
                             'src/bench.cpp',
                             dependencies:[eta_factorizer_dep, utils_dep])

