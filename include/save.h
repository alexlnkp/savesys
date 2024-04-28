#ifndef SAVE_H
#define SAVE_H

#include <time.h>
#include <stdbool.h>

// The size of buffer used to read the savefile
#define BUFFER_SIZE 128

typedef unsigned char uint8_t;
typedef unsigned int uint_least32_t;

struct Score {
    uint8_t D, M, Y; // Date of the score
    uint_least32_t score;
    char initials[4]; // 3-Letter initials
};

struct Save {
    uint_least32_t CheckSum;
    uint8_t D, M, Y; // Date of the save D:1|2, M:1|2, Y:2 (last two digits, i.e. 24 for 2024)
    uint8_t numScores;
    struct Score* scoreBoard;
};

/**
 * @brief Allocate memory for a new save structure
 *
 * Allocates memory for a new save structure, sets its date to 0/0/0, and
 * allocates memory for the scoreboard. If either allocation fails, the function
 * frees the memory it has allocated so far and returns NULL.
 *
 * @param numScores Number of scores to allocate space for in the scoreboard
 * @return Pointer to the new save structure on success, NULL on failure
 */
struct Save* newSave(size_t numScores) {
    struct Save* save = (struct Save*)malloc(sizeof(struct Save));
    if (save == NULL) return NULL; // If malloc fails, return NULL

    save->CheckSum = 0; // Initialize checksum to 0
    save->D = 0; // Initialize date to 0
    save->M = 0;
    save->Y = 0;
    save->numScores = numScores; // Set the amount of scores in the scoreboard array
    save->scoreBoard = (struct Score*)malloc(numScores * sizeof(struct Score));
    if (save->scoreBoard == NULL) { // If malloc fails, free the memory we have allocated and return NULL
        free(save);
        return NULL;
    }

    return save; // Return the pointer to the new save structure
}

// Checksum calculation function
uint_least32_t adler32(const uint8_t* data, size_t length) {
    uint_least32_t a = 1, b = 0;
    for (size_t i = 0; i < length; i++) {
        a = (a + data[i]) % 65521;
        b = (b + a) % 65521;
    }
    return ((b << 16) | a);
}

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
void* initSave(struct Score *Scores, size_t numScores) {
    // Allocate memory for the new save
    struct Save *save = newSave(numScores);

    // Get current date and time for the save-date
    time_t t = time(NULL);
    struct tm *now = localtime(&t);

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
    struct Save *readSave = (struct Save*)malloc(sizeof(struct Save)); {
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
        readSave->scoreBoard = (struct Score*)malloc(readSave->numScores * sizeof(struct Score));

        // Reading scoreboard
        for (int i = 0; i < readSave->numScores; ++i) {
            fread(&readSave->scoreBoard[i], sizeof(struct Score), 1, fp);
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

/**
 * @brief Writes the savefile to disk
 *
 * Writes the savefile to disk at the specified `path`.
 *
 * @param save The save structure that contains the data to be written
 * @param path The path where the savefile will be written
 * @return Returns 0 on success, non-zero otherwise.
 */
int makeSave(struct Save *save, const char* path) {
    {   //  Writing to savefile
        FILE* fp = fopen(path, "wb");
        if (!fp) return -1;

        // We don't actually need this variable here, it's needed just to write empty 4 bytes at the beginning of a file
        uint8_t emptyBytes[4] = {0, 0, 0, 0};
        fwrite(emptyBytes, sizeof(uint8_t), 4, fp);

        fwrite(&save->D, sizeof(save->D), 1, fp);
        fwrite(&save->M, sizeof(save->M), 1, fp);
        fwrite(&save->Y, sizeof(save->Y), 1, fp);

        fwrite(&save->numScores, sizeof(save->numScores), 1, fp);

        // Assuming we will always store 255 scores or less.
        for (uint8_t i = 0; i < save->numScores; i++) fwrite(&save->scoreBoard[i], sizeof(struct Score), 1, fp);

        fclose(fp);

    } { // Calculating checksum
        FILE* fp = fopen(path, "rb");

        uint8_t buffer[BUFFER_SIZE]; // Use a buffer to read the file in chunks
        size_t bytesRead = 0;
        uint_least32_t checksum = 1; // Initialize the checksum to a non-zero value

        while ((bytesRead = fread(buffer, sizeof(uint_least32_t), sizeof(buffer), fp)) > 0)
            checksum = adler32(buffer, bytesRead) ^ checksum;
        
        fclose(fp);
        
        save->CheckSum = checksum;

    } { // Writing checksum
        FILE* fp = fopen(path, "rb+");
        // Move the file pointer to the beginning of the file
        fseek(fp, 0, SEEK_SET);

        // Write the checksum to the first 4 bytes we previously left empty
        fwrite(&save->CheckSum, sizeof(save->CheckSum), 1, fp);
        fclose(fp);
    }
}

/**
 * @brief Adds a score to the savefile
 *
 * Reads the savefile at `savePath`, adds the given `AddScore`
 * to the scoreboard and updates the number of scores.
 *
 * @param AddScore The score to be added
 * @param savePath The path to the savefile
 * @return Returns 0 on success, non-zero otherwise.
 */
int AddScore(struct Score AddScore, const char* savePath) {
    // Read the save at the savePath
    struct Save *readSave = (struct Save*)ParseSave(savePath);
    
    // Increment the score amount in the save to allow for proper memory positioning
    uint8_t nS = readSave->numScores + 1;

    // Temporary struct Score array for adding the new Score in the savefile
    struct Score *t_sb = (struct Score*)malloc(nS * sizeof(struct Score));
    
    // Read existing scores on the scoreboard
    for (int i = 0; i < readSave->numScores; ++i) {
        // Copy existing score into the temporary scoreboard
        t_sb[i] = readSave->scoreBoard[i];
    }
    
    // Add the requested score onto the scoreboard
    t_sb[nS - 1] = AddScore;

    // Update the savefile's scoreboard and number of scores
    readSave->numScores = nS;
    readSave->scoreBoard = t_sb;

    // Save the savefile to disc
    return makeSave(readSave, savePath);
}

#endif