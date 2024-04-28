#ifndef SAVE_H
#define SAVE_H

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

struct Save* newSave(size_t numScores) {
    struct Save* save = (struct Save*)malloc(sizeof(struct Save));
    if (save == NULL) {
        return NULL;
    }

    save->CheckSum = 0;
    save->D = 0;
    save->M = 0;
    save->Y = 0;
    save->numScores = numScores;
    save->scoreBoard = (struct Score*)malloc(numScores * sizeof(struct Score));
    if (save->scoreBoard == NULL) {
        free(save);
        return NULL;
    }

    return save;
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

void* initSave(struct Score *Scores, size_t numScores);
void *ParseSave(const char* path);
_Bool CheckSum(const char* path, uint_least32_t StoredChSum);

#endif