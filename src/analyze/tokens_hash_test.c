#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define HASHSIZE 19645
#define TOKENSIZE 138078
#define HASHSIZE2 55511
int ttt[HASHSIZE][20] = {0};

int hash(const char *str) {
    const unsigned char *s = (unsigned char *)str;
    unsigned long hash = 44;
    int c;

    while (c = *s++) {
        hash = ((hash << 9) + hash) + c; /* hash * 33 + c */
    }

    return (int)(hash % HASHSIZE);
}

int hash2(const char *str, int size, int i, int j) {
    const unsigned char *s = (unsigned char *)str;
    unsigned long hash = 2687;
    int c;

    while (c = *s++) {
        hash = ((hash << j) + hash) + c; /* hash * 33 + c */
    }

    return (int)(hash % size);
}

int main() {
    char name[60];
    FILE *in_file = fopen("tokens.txt", "r");
    int max = 0;
    char tokens[TOKENSIZE][60];
    int i = 0;
    while (fscanf(in_file, "%s", tokens[i]) != EOF) {
        // printf("%s\n", name);
        i++;
    }
    // int global_min = 1000;
    fclose(in_file);
    // for (int i = 10000; i < 100000; i++) {
    //     for (int j = 0; j < 1; j++) {
    //         for (int k = 0; k < 1; k++) {
    // max = 0;
    // for (int tmp = 0; tmp < HASHSIZE; tmp++) {
    //     ttt[tmp] = 0;
    // }
    int success = 1;
    for (int size = 56001; size > 10000; size -= 2) {
        for (int j = 0; j < 32; j++) {
            for (int i = 0; i < HASHSIZE; i++) {
                for (int j = 0; j < 20; j++) {
                    ttt[i][j] = 0;
                }
            }
            success = 1;
            for (int token = 0; token < TOKENSIZE; token++) {
                int idx = hash(tokens[token]);
                int h2 = hash2(tokens[token], size, 0, j);
                int tmp = 0;
                while (ttt[idx][tmp] != 0) {
                    if (ttt[idx][tmp] == h2) {
                        success = 0;
                        break;
                    }
                    tmp++;
                }
                ttt[idx][tmp] = h2;
                if (!success) {
                    break;
                }
            }
            if (success) {
                printf("%d, j: %d\n", size, j);
            }
        }
    }
    // int cnt = 0;
    // for (int i = 0; i < HASHSIZE; i++) {
    //     int tmp = 0;
    //     while (ttt[i][tmp] != 0) {
    //         cnt++;
    //         printf("%d ", ttt[i][tmp]);
    //         tmp++;
    //     }
    //     printf("\n");
    // }
    // printf("total: %d\n", cnt);
    // if (max < global_min) {
    //     global_min = max;
    //     printf("global min: %d, i: %d, j: %d, k: %d\n", global_min, i, j,
    //     k);
    // }
    // }
    // }
    // }

    // printf("max: %d\n", max);
};