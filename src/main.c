#include "api.h"
#define HASHSIZE 10461365
// #define INDEXSIZE 1000
// #define HASHSIZE2 2147483647
#define MAX_NMAILS 10000
#define TOKENSIZE 138078
#define BITMAP_LEN 17260
// ========================================
// Token set implementation

typedef struct Node {
    struct Node *next;
    // int hash;
    int hash2;
    int num;
    // char *s;
} Node;

typedef struct {
    Node **hash_table;
    // Node *items;
    int n_items;
} GlobalTokenSet;

typedef struct {
    int n_items;
    unsigned char bitmap[17260];
} TokenSet;
// Hash function for string
// Reference:
// https://stackoverflow.com/questions/7666509/hash-function-for-string
int hash(const char *str) {
    const unsigned char *s = (unsigned char *)str;
    unsigned long hash = 44;
    int c;

    while (c = *s++) {
        hash = ((hash << 9) + hash) + c;
        // printf("%c, %d\n", c, c);
    } /* hash * 33 + c */

    return (int)(hash % HASHSIZE);
}

int hash2(const char *str) {
    const unsigned char *s = (unsigned char *)str;
    unsigned long hash = 2687;
    int c;

    while (c = *s++) hash = ((hash << 11) + hash) + c; /* hash * 33 + c */

    return (int)(hash);
}

// See whether token in the token set or not
// bool SetContains(TokenSet *set, int h) {  // const char *token,
//     // int offset = h % 8;
//     // int idx = h >> 3;
//     if (set->bitmap[h >> 3] & (1 << (h % 8)) == 0) {
//         return false;
//     }
//     return true;
// }

// ========================================
// Global variables

int n_mails, n_queries;
mail *mails;
query *queries;
int token_num = 0;
// int max_collision = 0;

int answer[MAX_NMAILS];

TokenSet tokensets[MAX_NMAILS] = {NULL};

// int *hash_table_cnt;
GlobalTokenSet global_tokenset;

double similaritys[MAX_NMAILS][MAX_NMAILS] = {0};

// ========================================

int GlobalSetContains(int h, int h2) {
    Node *node = global_tokenset.hash_table[h];
    // if (!strcmp("175", token)) {
    //     printf()
    // }
    while (node != NULL) {
        if (h2 == node->hash2) {
            return node->num;
        }
        node = node->next;
    }
    return -1;
}

// Add a token into the token set
int GlobalSetAdd(char *token) {  // const char *token,
    int h = hash(token);
    int h2 = hash2(token);

    int num = GlobalSetContains(h, h2);
    if (num != -1) {
        return num;
    }
    // if (!strcmp(token, "175")) {
    //     printf("token: %s, h: %d, h2: %d\n", token, h, h2);
    // }
    Node *hash_node = (Node *)malloc(sizeof(Node));
    Node *item_node = (Node *)malloc(sizeof(Node));

    hash_node->next = global_tokenset.hash_table[h];
    // hash_node->hash = h;
    hash_node->hash2 = h2;
    hash_node->num = token_num;
    // hash_node->s = token;
    global_tokenset.hash_table[h] = hash_node;

    // item_node->next = global_tokenset.items;
    // item_node->hash = h;
    // item_node->hash2 = h2;
    // item_node->num = token_num;
    // item_node->s = token;
    // global_tokenset.items = item_node;
    ++(global_tokenset.n_items);
    token_num++;
    // hash_table_cnt[h]++;
    // printf("%s\n", token);
    return hash_node->num;
}

// void parse_and_add_to_global_set(char *s) {
//     char *start = NULL;
//     char c;
//     while (c = *s) {
//         if (c >= 'A' && c <= 'Z') {
//             c = *s = c - 'A' + 'a';  // convert to lowercase
//         }
//         if (!((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9'))) {
//             if (start) {
//                 *s = '\0';
//                 int num = GlobalSetAdd(start);

//                 start = NULL;
//             }
//         } else if (!start) {
//             start = s;
//         }
//         ++s;
//     }
//     if (start) GlobalSetAdd(&global_tokenset, start);  // start,
// }

void parse_and_add_to_token_set(char *s, TokenSet *set) {
    char *start = NULL;
    char c;
    unsigned int num;
    while (c = *s) {
        if (c >= 'A' && c <= 'Z') {
            c = *s = c - 'A' + 'a';  // convert to lowercase
        }
        if (!((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9'))) {
            if (start) {
                *s = '\0';
                num = GlobalSetAdd(start);
                // printf("%s, num: %d\n", start, num);
                // printf("%d\n",
                //    (unsigned int)set->bitmap[num >> 3] & (1 << (num %
                //    8)));
                if ((set->bitmap[num >> 3] & (1 << (num % 8))) == 0) {
                    // printf("here\n");
                    set->n_items++;
                    // printf("%s, num: %d, num//8: %d\n", start, num, num >>
                    // 3); printf("%d ", set->bitmap[num >> 3]);
                    set->bitmap[num >> 3] |= (1 << (num % 8));
                    // printf("%d\n", set->bitmap[num >> 3]);
                }
                start = NULL;
            }
        } else if (!start) {
            start = s;
        }
        ++s;
    }
    if (start) {
        num = GlobalSetAdd(start);
        if (set->bitmap[num >> 3] & (1 << (num % 8)) == 0) {
            set->n_items++;
            set->bitmap[num >> 3] |= 1 << (num % 8);
        }
    }
}

double context_similarity(int i, int j) {
    Node *node;
    int n_intersection = 0;
    int n;
    // if (tokensets[i].n_items > tokensets[j].n_items) {
    //     temp = i;
    //     i = j;
    //     j = temp;
    // }
    // node = tokensets[i].items;
    for (int tmp = 0; tmp < BITMAP_LEN; tmp++) {
        n = ((u_int8_t)(tokensets[i].bitmap[tmp]) &
             (u_int8_t)(tokensets[j].bitmap[tmp]));
        while (n != 0) {
            n = n & (n - 1);
            n_intersection++;
        }
    }
    // n_intersection = 0;
    // while (node) {
    //     if (SetContains(tokensets + j, node->hash, node->hash2))  // node->s,
    //         ++n_intersection;
    //     node = node->next;
    // }
    // n_intersection += global_tokenset.n_items;
    return (double)(n_intersection) /
           (tokensets[i].n_items + tokensets[j].n_items - n_intersection);
}

void find_similar_query(int query_id, int mail_id, double threshold) {
    int i, answer_length;
    double sim;
    answer_length = 0;
    for (i = 0; i < n_mails; ++i) {
        if (i == mail_id) continue;
        if (similaritys[i][mail_id] != 0) {
            sim = similaritys[i][mail_id];
        } else {
            sim = context_similarity(i, mail_id);
            similaritys[i][mail_id] = sim;
            similaritys[mail_id][i] = sim;
        }
        if (sim > threshold) {
            answer[answer_length++] = i;
        }
    }
    api.answer(query_id, answer, answer_length);
}

// ========================================

int main(void) {
    api.init(&n_mails, &n_queries, &mails, &queries);
    clock_t start = clock();
    int i;
    // char common[45] = "I read the paragraph on http://wikipedia.org ";
    global_tokenset.hash_table = malloc(HASHSIZE * sizeof(Node *));
    // hash_table_cnt = malloc(HASHSIZE * sizeof(int));
    // parse_and_add_to_global_set(common);
    for (i = 0; i < n_mails; ++i) {
        // for (int j = 0; j < n_mails; ++j) {
        //     similaritys[i][j] = -1;
        // }
        // tokensets[i].hash_table = malloc(HASHSIZE * sizeof(Node *));
        parse_and_add_to_token_set(mails[i].subject, tokensets + mails[i].id);
        parse_and_add_to_token_set(mails[i].content, tokensets + mails[i].id);
    }
    // printf("%d\n", clock() - start);
    // printf("global_n_itmes: %d, offset_num: %d\n", global_tokenset.n_items,
    //        token_num);
    // printf("tokensets[0].items: %d\n", tokensets[0].n_items);
    // for (int kkk = 0; kkk < 13; kkk++) {
    //     printf("%d", tokensets[0].bitmap[kkk]);
    // }
    double score = 0;
    for (i = 0; i < n_queries; ++i) {
        if (queries[i].type == find_similar && queries[i].reward >= 100) {
            find_similar_query(queries[i].id,
                               queries[i].data.find_similar_data.mid,
                               queries[i].data.find_similar_data.threshold);
            score += queries[i].reward;
            fprintf(stderr, "%f\n", score);
        }
    }

    return 0;
}
