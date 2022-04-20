#include "prediction.hpp"

#include <iostream>

#include "debug.hpp"

auto serialize(Terminal const * t, std::ostream & os) -> void {
    auto const e = static_cast<Event const *>(t->payload);
    switch (e->operation) {
        case Operation::Allocation: {
            os << 'A' << e->dest;
        } break;
        case Operation::Deallocation: {
            os << 'D' << e->orig;
        } break;
        case Operation::Reallocation: {
            os << 'R' << e->orig << "->" << e->dest;
        } break;
        default: check(false); break;
    }
}

auto deserialize(std::istream & is, size_t size) -> void * {
    char buf[2048];
    check(size < 2048);
    is.read(buf, size);
    auto const e = new Event {};
    switch (buf[0]) {
        case 'A': {
            e->operation = Operation::Allocation;
            e->orig = 0;
            sscanf(buf + 1, "%lu", &e->dest);
        } break;
        case 'D': {
            e->operation = Operation::Deallocation;
            sscanf(buf + 1, "%lu", &e->orig);
            e->dest = 0;
        } break;
        case 'R': {
            e->operation = Operation::Reallocation;
            sscanf(buf + 1, "%lu->%lu", &e->orig, &e->dest);
        } break;
        default: check(false); break;
    }
    return e;
}

