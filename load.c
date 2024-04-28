#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "save.h"

#define C struct

int main() {
    C Save *readSave = (C Save*)ParseSave("save.sf"); 
    
    for (int i = 0; i < readSave->numScores; i++) {
        printf("%d: %d %s %d/%d/%d\n", i, readSave->scoreBoard[i].score, readSave->scoreBoard[i].initials, readSave->scoreBoard[i].D, readSave->scoreBoard[i].M, readSave->scoreBoard[i].Y);
    }

    return 0;
}