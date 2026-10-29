#include <assert.h>
#include <string.h>
#include "money.h"
int main(void) {
    char out[64];
    TabbozFormatMoney(0, out, sizeof(out)); assert(!strcmp(out, "0,00 EUR"));
    TabbozFormatMoney(1, out, sizeof(out)); assert(!strcmp(out, "0,50 EUR"));
    TabbozFormatMoney(2, out, sizeof(out)); assert(!strcmp(out, "1,00 EUR"));
    TabbozFormatMoney(199, out, sizeof(out)); assert(!strcmp(out, "99,50 EUR"));
    TabbozFormatMoney(1000, out, sizeof(out)); assert(!strcmp(out, "500,00 EUR"));
    TabbozFormatMoney(0xffffffffUL, out, sizeof(out)); assert(!strcmp(out, "2147483647,50 EUR"));
    return 0;
}
