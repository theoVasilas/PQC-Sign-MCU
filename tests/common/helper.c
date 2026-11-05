#include <stdio.h>
#include "helper.h"

void print_hex(const char *label, const uint8_t *data, size_t len) {
    printf("%s (%zu bytes):\n", label, len);

    for (size_t i = 0; i < len; i++) {
        printf("%02X", data[i]);

        /* Print 16 bytes per line for readability */
        if ((i + 1) % 16 == 0)
            printf("\n");
        else
            printf(" ");
    }

    printf("\n\n");
}

