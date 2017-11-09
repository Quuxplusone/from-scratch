
#include "text.h"
#include <stdio.h>

int main()
{
    scratch::text t("héllö \u0301w☃r\x80\x80ld\xE2");

    printf("%s\n", t.str().c_str());

    for (auto&& cu : t.code_units()) {
        printf("%02x\n", (uint8_t)cu);
    }
    printf("\n");

    for (auto&& cp : t.code_points()) {
        printf("U+%04X\n", (uint32_t)cp);
    }
    printf("\n");

    for (auto&& g : t.graphemes()) {
        printf("[");
        for (auto&& cp : g.code_points()) {
            printf(" U+%04X ", cp);
        }
        printf("]");
    }
    printf("\n");
}
