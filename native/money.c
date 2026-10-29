#include "money.h"
#include <stdio.h>
void TabbozFormatMoney(unsigned long units, char *output, size_t capacity) {
    if (capacity) snprintf(output, capacity, "%lu,%02lu EUR", units / 2, (units % 2) * 50);
}
