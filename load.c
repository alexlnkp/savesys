#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "save.h"

#define C struct

/**
 * @brief Calculate checksum and compare with the one written in the file
 *
 * This function calculates the checksum of the savefile, reads the checksum
 * that is stored in the first 4 bytes of the file, then compares the two.
 * If the checksums match, the function returns false, otherwise it
 * returns true.
 *
 * @param path Path to the savefile
 * @param StoredChSum Checksum stored in the savefile's first 4 bytes
 * @return True if checksum comparison didn't pass, False if it did
 */
bool CheckSum(const char* path, uint_least32_t StoredChSum) {
    uint_least32_t chsum; {
        // Calculate checksum of the savefile
        FILE* fp = fopen(path, "rb+");

        // Zero-out the checksum to not throw off our calculations
        fseek(fp, 0L, SEEK_SET);
        uint8_t emptyBytes[4] = {0, 0, 0, 0};
        fwrite(emptyBytes, sizeof(uint8_t), 4, fp);

        // Use a buffer to read the file in chunks
        uint8_t buffer[BUFFER_SIZE];
        size_t bytesRead = 0;
        // Initialize the checksum to a non-zero value
        uint_least32_t checksum = 1;

        // Reset the file pointer to the beginning
        fseek(fp, 0L, SEEK_SET);

        // Calculate the checksum of the file
        while ((bytesRead = fread(buffer, sizeof(uint_least32_t), sizeof(buffer), fp)) > 0)
            checksum = adler32(buffer, bytesRead) ^ checksum;

        // Store the calculated checksum in a variable
        chsum = checksum;

        // Insert original checksum back in the file
        fseek(fp, 0L, SEEK_SET);
        fwrite(&StoredChSum, sizeof(uint_least32_t), 1, fp);

        fclose(fp);
    }

    // Return true if the calculated checksum and stored checksum are different
    return (chsum != StoredChSum);
}

/**
 * @brief Parse the savefile
 * @param path Path to the savefile, if NULL uses "save.sf"
 * @return Pointer to the parsed save or NULL if it fails
 */
void *ParseSave(const char* path) {
    C Save *readSave = (C Save*)malloc(sizeof(C Save)); {
        // Reading savefile
        if (!path) path = "save.sf";
        FILE* fp = fopen(path, "rb");
        if (!fp) return NULL;

        // Reading first 4bytes for the checksum
        fread(&readSave->CheckSum, sizeof(readSave->CheckSum), 1, fp);

        // Reading date
        fread(&readSave->D, sizeof(readSave->D), 1, fp);
        fread(&readSave->M, sizeof(readSave->M), 1, fp);
        fread(&readSave->Y, sizeof(readSave->Y), 1, fp);

        // Reading number of scores
        fread(&readSave->numScores, sizeof(readSave->numScores), 1, fp);

        // Allocate memory for the scoreboard
        readSave->scoreBoard = (C Score*)malloc(readSave->numScores * sizeof(C Score));

        // Reading scoreboard
        for (int i = 0; i < readSave->numScores; ++i) {
            fread(&readSave->scoreBoard[i], sizeof(C Score), 1, fp);
        }

        fclose(fp);
    }

    // Check the 'sum
    if (CheckSum(path, readSave->CheckSum)) {
        printf("CHECKSUM FAILED! SAVEFILE HAS BEEN TAMPERED WITH.\n");
        return NULL;
    }
    return readSave;
}

int main() {
    C Save *readSave = (C Save*)ParseSave("save.sf"); 
    
    for (int i = 0; i < readSave->numScores; i++) {
        printf("%d: %d %s %d/%d/%d\n", i, readSave->scoreBoard[i].score, readSave->scoreBoard[i].initials, readSave->scoreBoard[i].D, readSave->scoreBoard[i].M, readSave->scoreBoard[i].Y);
    }

    return 0;
}