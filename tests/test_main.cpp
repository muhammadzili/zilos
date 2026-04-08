#include <cstdio>

extern "C" void kernel_main();

namespace test {
    void run_all() {
        printf("[TEST] Running ZilOS tests...\n");
    }
}
