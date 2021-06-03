#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define HASHSIZE 19697
int ttt[HASHSIZE] = {0};
/// 7 27 148
// unsigned long ans = 148;
// ans = ((ans << 7) + ans + (change(s[i]) << 27));

// ans = ((ans << 0) + (change(s[i]) << 2) + ((ans * ans) << 16) +
// ((change(s[i]) * change(s[i])) << 24));
int change(char s) {
    if (s >= 'A' && s <= 'Z') {
        return s - 'A';
    } else {
        return s - 'a';
    }
};

unsigned int hash(char *s, int l, int j, int k) {
    unsigned long ans = k;
    int i = 0;
    while (i < strlen(s)) {
        ans = ((ans << l) + ans + (change(s[i]) << j));
        //+((ans * ans) << 16)+((change(s[i]) * change(s[i])) << 24)
        i++;
    }
    return (ans % HASHSIZE);
};

int main() {
    char name[50];
    FILE *in_file = fopen("users.txt", "r");
    int cnt = 0;
    char users[560][50];
    int i = 0;
    while (fscanf(in_file, "%s", users[i]) != EOF) {
        // printf("%s\n", name);
        i++;
    }
    fclose(in_file);
    // for (int i = 0; i < 560; i++)
    // {
    //     printf("%s\n", users[i]);
    // }
    int min = 100;
    int mins[4] = {0, 0, 0, 0};
    for (int i = 0; i < 32; i++) {
        for (int j = 0; j < 32; j++) {
            for (int k = 0; k < 10000; k++) {
                //             for (int l = 0; l < 32; l++)
                //             {
                cnt = 0;
                for (int index = 0; index < HASHSIZE; index++) {
                    ttt[index] = 0;
                }
                for (int user = 0; user < 560; user++) {
                    unsigned int idx = hash(users[user], i, j, k);
                    if (ttt[idx] == 1) {
                        cnt++;
                        if (cnt >= min) {
                            break;
                        }
                    } else {
                        ttt[idx] = 1;
                    }
                }
                if (cnt < min) {
                    mins[0] = i;
                    mins[1] = j;
                    mins[2] = k;
                    // mins[3] = l;
                    min = cnt;
                    printf("min: %d ", min);
                    // if (min == 0) {
                    for (int i = 0; i < 3; i++) {
                        printf("%d ", mins[i]);
                    }
                    printf("\n");
                    // }
                }
                //             }
            }
        }
    }

    // for (int user = 0; user < 560; user++) {
    //     unsigned int idx = hash(users[user]);
    //     if (ttt[idx] == 1) {
    //         printf("fail\n");
    //         break;
    //     } else {
    //         ttt[idx] = 1;
    //     }
    // }
};