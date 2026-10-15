#include "py/mphal.h"

int mp_hal_stdin_rx_chr(void) {
    return getc(stdin);
}

mp_uint_t mp_hal_stdout_tx_strn(const char *str, size_t len) {
    return fwrite(str, len, 1, stdout) == 1 ? len : 0;
}
