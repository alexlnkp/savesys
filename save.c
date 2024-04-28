#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "save.h"

#define ARR_LEN(A) sizeof(A)/sizeof(A[0])

#define C struct

int main() {
    C Score newscore = {27, 4, 24, 102030, "AMK"};
    C Score difscore = {25, 2, 23, 101010, "DBJ"};

    C Score Scores[] = {
        newscore, difscore
    };

    C Save *save = newSave(ARR_LEN(Scores));
    save->D = 27;
    save->M = 4;
    save->Y = 24;
    
    save->numScores = ARR_LEN(Scores);

    for (int i = 0; i < ARR_LEN(Scores); i++) save->scoreBoard[i] = Scores[i];

{   //  Writing to savefile
    FILE* fp = fopen("save.sf", "wb");
    if (!fp) return -1;

    // We don't actually need this variable here, it's needed just to write empty 4 bytes at the beginning of a file
    uint8_t emptyBytes[4] = {0, 0, 0, 0};
    fwrite(emptyBytes, sizeof(uint8_t), 4, fp);

    fwrite(&save->D, sizeof(save->D), 1, fp);
    fwrite(&save->M, sizeof(save->M), 1, fp);
    fwrite(&save->Y, sizeof(save->Y), 1, fp);

    fwrite(&save->numScores, sizeof(save->numScores), 1, fp);

    for (int i = 0; i < save->numScores; i++) fwrite(&save->scoreBoard[i], sizeof(C Score), 1, fp);

    fclose(fp);
}

{   // Calculating checksum
    FILE* fp = fopen("save.sf", "rb");

    uint8_t buffer[BUFFER_SIZE]; // Use a buffer to read the file in chunks
    size_t bytesRead = 0;
    uint_least32_t checksum = 1; // Initialize the checksum to a non-zero value

    while ((bytesRead = fread(buffer, sizeof(uint_least32_t), sizeof(buffer), fp)) > 0)
        checksum = adler32(buffer, bytesRead) ^ checksum;
    
    fclose(fp);
    
    save->CheckSum = checksum;
}

{   // Writing checksum
    FILE* fp = fopen("save.sf", "rb+");
    // Move the file pointer to the beginning of the file
    fseek(fp, 0, SEEK_SET);

    // Write the checksum to the first 4 bytes we previously left empty
    fwrite(&save->CheckSum, sizeof(save->CheckSum), 1, fp);
    fclose(fp);
}


    return 0;
}