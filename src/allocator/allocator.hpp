#pragma once

#include <stdlib.h>

#include "settings.hpp"

namespace Allocator {

auto malloc(size_t size) -> void *;
auto realloc(void * ptr, size_t size) -> void *;
auto calloc(size_t nmemb, size_t size) -> void *;
auto free(void *) -> void;

auto init(Settings const &) -> void;
auto deinit(Settings const &) -> void;

}  // namespace Allocator

