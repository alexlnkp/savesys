#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "save.h"

int main() {
    AddScore((struct Score){21, 1, 20, 205080, "BBJ"}, "save.sf");

    return 0;
}