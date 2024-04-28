#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "save.h"

#define ARR_LEN(A) sizeof(A)/sizeof(A[0])

#define C struct

/**
 * @brief Initialize save with given scores
 *
 * Allocates memory for a new save, copies given scores to it,
 * sets date in the struct to the current date and returns a pointer to the structure.
 *
 * @param Scores Array of scores to be copied to the new save
 * @param numScores Number of scores in the `Scores` array
 * @return Pointer to the new save
 */
void* initSave(C Score *Scores, size_t numScores) {
    // Allocate memory for the new save
    C Save *save = newSave(numScores);

    // Get current date and time for the save-date
    time_t t = time(NULL);
    C tm *now = localtime(&t);

    // Set date in the struct to the current date
    // Format: save->D = 28; save->M = 4; save->Y = 24;
    save->D = now->tm_mday;
    save->M = now->tm_mon + 1;
    save->Y = now->tm_year % 100;

    // Set the number of scores
    save->numScores = numScores;

    // Copy scores from `Scores` array to the new save
    for (int i = 0; i < numScores; i++) save->scoreBoard[i] = Scores[i];

    // Return the pointer to the new save
    return save;
}


int makeSave(C Save *save) {
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

        // Assuming we will always store 255 scores or less.
        for (uint8_t i = 0; i < save->numScores; i++) fwrite(&save->scoreBoard[i], sizeof(C Score), 1, fp);

        fclose(fp);
    } {   // Calculating checksum
        FILE* fp = fopen("save.sf", "rb");

        uint8_t buffer[BUFFER_SIZE]; // Use a buffer to read the file in chunks
        size_t bytesRead = 0;
        uint_least32_t checksum = 1; // Initialize the checksum to a non-zero value

        while ((bytesRead = fread(buffer, sizeof(uint_least32_t), sizeof(buffer), fp)) > 0)
            checksum = adler32(buffer, bytesRead) ^ checksum;
        
        fclose(fp);
        
        save->CheckSum = checksum;
    } {   // Writing checksum
        FILE* fp = fopen("save.sf", "rb+");
        // Move the file pointer to the beginning of the file
        fseek(fp, 0, SEEK_SET);

        // Write the checksum to the first 4 bytes we previously left empty
        fwrite(&save->CheckSum, sizeof(save->CheckSum), 1, fp);
        fclose(fp);
    }
}

int main() {
    C Score Scores[] = {
        {27, 4, 24, 102030, "AMK"},
        {25, 2, 23, 101010, "DBJ"}
    };

    C Save* save = (C Save*)initSave(Scores, ARR_LEN(Scores));

    makeSave(save);

    return 0;
}