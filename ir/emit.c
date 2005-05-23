#include "ir.h"

int main(void) {
    struct ir_s * ir;

    ir = ir_new();

    ir_free(ir);
    return 0;
}
