#ifndef UTILS_H_
#define UTILS_H_

#include <stdio.h>
#include <stdlib.h>

#define assert_nonnull(e, message)                                             \
    do {                                                                       \
        if ((e) == NULL) {                                                     \
            fputs ((message), stderr);                                         \
            exit (EXIT_FAILURE);                                               \
        }                                                                      \
    } while (0)

#endif
