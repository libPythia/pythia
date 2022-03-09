#include "stdlib.h"

int main() {
    auto volatile ptr = malloc(20);
    ptr = realloc(ptr, 40);
    free(ptr);
}

