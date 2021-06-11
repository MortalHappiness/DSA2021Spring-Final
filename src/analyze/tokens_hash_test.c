

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define HASHSIZE 94482925 // 2147483647
// 293741823                    // 2147483647  // 4294967296  // 10483647
// //2147483647
#define TOKENSIZE 138078
#define USERSIZE 560
// #define HASHSIZE2 55511
// #define UNSIGNED 2147483647
// int ttt[HASHSIZE] = {0};

typedef struct Node {
    struct Node *next;
    // const char *key;
    int len;
} Node;
int hash(const char *str, int i) {
    const unsigned char *s = (unsigned char *)str;
    unsigned long hash = 44;
    int c;
    int offset = 0;
    while (c = *s++) {
        hash = ((hash << 7) + (hash)) + c; /* hash * 33 + c */
        // printf("%c %d\n", c, c);
    }

    return (int)(hash % i);
}

int hash2(const char *str, int i) {
    const unsigned char *s = (unsigned char *)str;
    int len = strlen(s);
    unsigned long hash = 2687;
    int c;
    unsigned int *k;

    while (len >= 4) {
        k = (unsigned int *)s;
        // *k *= 0xcc9e2d51;
        // *k = (*k << 11) | (*k >> 21);
        // *k *= 0x1b873593;
        hash ^= *k + hash;
        hash = ((hash << i) | (hash >> (32 - i))); /* hash * 33 + c */
        // hash = hash * 5 + 0xe6546b64;
        len -= 4;
        s += 4;
    }
    *k = 0;
    switch (len) {
    case 3:
        *k = s[2] << 16;
    case 2:
        *k |= s[1] << 8;
    default:
        *k |= s[0];
    }
    hash ^= *k;
    hash = ((hash << i) | (hash >> (32 - i)));
    hash ^= hash >> 16;
    // hash *= 0x85ebca6b;
    // hash ^= hash >> 13;
    // hash *= 0xc2b2ae35;
    // hash ^= hash >> 16;
    // has
    return (int)(hash % HASHSIZE);
}

int hash3(const char *str) {
    const unsigned char *s = (unsigned char *)str;
    unsigned long hash = 0;
    int c;
    int offset = 0;
    while (c = *s++) {
        hash = ((hash << 7) | (hash >> 25)) ^ c; /* hash * 33 + c */
        // printf("%c %d\n", c, c);
    }

    return (int)(hash % HASHSIZE);
}

int hash4(const char *str, int len) {
    // char s[5];
    // strcpy(s, str);
    unsigned int *k;
    k = (unsigned int *)str;
    printf("%s %08x\n", str, *k);
    unsigned int hash = (*k << ((3 - (strlen(str) - 1) % 4) * 8));
    hash = (hash) ^ (hash >> 15);
    return (int)(hash % 4727341);
}

int hash5(const char *str, int i) {
    // char s[9];
    // const unsigned char *s = (unsigned char *)str;
    // strcpy(s, str);
    // int len = strlen(s);
    // unsigned long hash = 2687;
    // int c;
    unsigned int tmp;
    // k = (unsigned int *)str;
    unsigned int hash = *(unsigned int *)str;
    printf("%s %08x\n", str, hash);
    int turn = hash & 3;
    hash = (hash << (11 + turn)) ^ hash ^ (hash >> (21 - turn));
    tmp = *(unsigned int *)(str + 4);
    tmp = (tmp << ((3 - (strlen(str) - 1) % 4) * 8));
    turn += hash & 3;
    hash = hash ^ (tmp) ^ (tmp >> (11 - turn)); //<< (7 + turn)
    turn += hash & 3;
    hash = hash ^ (hash >> (11 + turn));
    turn += hash & 3;
    // hash = hash ^ (hash << (5 + turn));
    hash = hash ^ (tmp >> (7 + turn));
    return (int)(hash % 294560837);
}

int hash6(const char *str, int len, int size) {
    // char *s = malloc(57 * sizeof(char));
    // char *tmp = s;
    // const unsigned char *s = (unsigned char *)str;
    // strcpy(s, str);
    // int len = strlen(str);
    // int len = strlen(s);
    // unsigned long hash = 2687;
    // int c;
    // s += 4;
    // len -= 4;
    unsigned int tmp;
    // k = (unsigned int *)str;
    // printf("%s %08x\n", str, *k);
    tmp = *(unsigned int *)str;
    unsigned int hash = tmp;

    int i = 0;
    while (len > 3) {
        hash = (tmp << (11 + i)) ^ hash ^ (tmp >> (21 - i));
        str = (str + 4);
        // k = (unsigned int *)(str);
        tmp = *(unsigned int *)(str);
        len -= 4;
        i = i + (hash & 3);
        i = i % 11;
    }
    if (len > 0) {
        tmp = (tmp << ((4 - len) * 8));
        hash = (tmp) ^ hash ^ (tmp >> (11 - i));
        // free(tmp);
        i += hash & 3;
        i = i % 11;
    }
    hash = hash ^ (hash >> (15 + i));
    return (int)(hash % size); //
}
int main() {
    char name[60];
    // printf("sizeof long long int: %d, long int: %d, long: %d\n",
    //        sizeof(unsigned long long), sizeof(long int), sizeof(long));
    FILE *in_file = fopen("tokens.txt", "r");
    int max;
    char tokens[TOKENSIZE][60];
    int **ttt = malloc(HASHSIZE * sizeof(int *));
    int *top = malloc(HASHSIZE * sizeof(int));
    Node **tokenset = malloc(HASHSIZE * sizeof(Node *));
    Node *node, *delete;
    int i = 0;
    int len;
    while ((fscanf(in_file, "%s", tokens[i])) != EOF) {
        i++;
    }
    fclose(in_file);
    // char **hashtable = malloc(HASHSIZE * sizeof(char *));
    int global_min = 1000;
    // for (int i = 10000; i < 100000; i++) {
    //     for (int j = 0; j < 1; j++) {
    //         for (int k = 0; k < 1; k++) {
    // max = 0;
    for (int tmp = 0; tmp < HASHSIZE; tmp++) {
        ttt[tmp] = malloc(2 * sizeof(int));
    }
    // int success = 1;
    // printf("start\n");
    for (int i = HASHSIZE; i > 0; i -= 2) {
        // char **hashtable = malloc(HASHSIZE * sizeof(char *));
        // for (int j = 0; j < 32; j++) {
        //     for (int l = 0; l < 32; l++) {
        // for (int k = 0; k < i; k++) {
        //     node = tokenset[k];
        //     while (node != NULL) {
        //         delete = node;
        //         node = node->next;
        //         free(delete);
        //     }
        // }
        for (int k = 0; k < i; k++) {
            // for (int l = 0; l < 2; l++) {
            //     ttt[k][l] = 0;
            // }
            top[k] = 0;
        }
        int success = 1;
        max = 0;
        // printf("token\n");
        // int cnt = 0;
        for (int token = 0; token < TOKENSIZE; token++) {
            // if (strlen(tokens[token]) > 8) {  //&& strlen(tokens[token]) <= 8
            // printf("%s\n", tokens[token]);
            int len = strlen(tokens[token]);
            int idx = hash(tokens[token], i);
            for (int j = 0; j < top[idx]; j++) {
                if (ttt[idx][j] == len) {
                    success = 0;
                    break;
                }
            }
            if (top[idx] >= 2) {
                // printf(:)
                max = 3;
                success = 0;
                break;
            }
            ttt[idx][top[idx]] = len;
            top[idx]++;
            // node = tokenset[idx];
            // while (node != NULL) {
            //     if (node->len == len) {
            //         success = 0;
            //         break;
            //     }
            //     node = node->next;
            // }
            // node = malloc(sizeof(Node));
            // node->len = len;
            // node->next = tokenset[idx];
            // tokenset[idx] = node;

            // if (ttt[idx] > max) {
            //     max = ttt[idx];
            // }
            // if (ttt[idx] > 1) {
            // printf("fail\n");
            // success = 0;
            // break;
            // max++;
            // if (max > global_min) {
            //     break;
            // }
            // printf("%s %s\n", tokens[token], hashtable[idx]);
            // printf("max: %d\n", max);
            // }
            // hashtable[idx] = tokens[token];
            // }
        }
        if (success) {
            printf("success %d max: %d\n", i, max);
        }
        // break;
        // if (i == 573741823) {
        //     printf("cnt: %d\n", cnt);
        // }
        // }
        // if (max <= global_min) {
        //     global_min = max;
        //     printf("max: %d, size: %d, j: %d, l: %d\n", max, i, 0,
        //            0); //, i: %d, j: %d
        // }
        // break;
        // printf(
        //     "------------------------------------------------------------------"
        //     "---------\n");
        // printf(
        //     "------------------------------------------------------------------"
        //     "---------\n");
        // printf(
        //     "------------------------------------------------------------------"
        //     "---------\n");
        // printf(
        //     "------------------------------------------------------------------"
        //     "---------\n");
        // if ((i - 1) % 100000 == 0) {
        //     printf("%d\n", i);
        //     }
        // }
    }
    // int cnt = 0;
    // for (int i = 0; i < HASHSIZE; i++) {
    //     int tmp = 0;
    //     while (ttt[i][tmp] != 0) {
    //         cnt++;
    //         // printf("%d ", ttt[i][tmp]);
    //         tmp++;
    //     }
    //     // printf("\n");
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