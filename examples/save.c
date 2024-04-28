#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "save.h"

#define ARR_LEN(A) sizeof(A)/sizeof(A[0])

#define C struct

int main() {
    C Score Scores[] = {
        {27, 4, 24, 102030, "AMK"},
        {25, 2, 23, 101010, "DBJ"}
    };

    C Save* save = (C Save*)initSave(Scores, ARR_LEN(Scores));

    makeSave(save, "save.sf");

    return 0;
}