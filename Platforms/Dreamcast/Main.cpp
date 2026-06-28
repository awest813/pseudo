#include "Global.h"

int main(int argc, char **argv) {
    (void)argc;
    (void)argv;

#if defined(__KOS__)
    printf("PSeudo Dreamcast scaffold ready\n");
#else
    std::puts("PSeudo Dreamcast scaffold ready");
#endif

    return 0;
}
