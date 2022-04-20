#pragma once

#include <stdlib.h>

auto eta_tmp_malloc(size_t size) -> void *;
auto eta_tmp_free(void * ptr) -> void;
