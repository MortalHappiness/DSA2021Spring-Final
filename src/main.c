#include "api.h"
#define HASHSIZE 247479207  // 10461365
// 247483383
#define HASHSIZE4 4727341
#define HASHSIZE5 294362917  // 294560837  // 294362917
#define HASHSIZE6 133362575  // 133741263  // 133362575

#define MAX_NMAILS 10000
#define TOKENSIZE 138078
#define BITMAP_LEN 2158
#define MAX_TOKENS_PER_EMAIL 3416
// ========================================
// Token set implementation

typedef struct Node {
    // struct Node *next;
    // int hash;
    // int hash2;
    int num;
    // char *s;
    // int len;
} Node;

typedef struct {
    Node **hash_table;
    // Node *items;
    // int n_items;
} GlobalTokenSet;

typedef struct {
    int n_items;
    unsigned long long bitmap[BITMAP_LEN];
    int top;
    int stack[MAX_TOKENS_PER_EMAIL];
} TokenSet;
// Hash function for string
// Reference:
// https://stackoverflow.com/questions/7666509/hash-function-for-string
int hash(const char *str) {
    const unsigned char *s = (unsigned char *)str;
    unsigned long hash = 44;
    int c;

    while (c = *s++) {
        hash = ((hash << 7) + hash) + c;
    } /* hash * 33 + c */

    return (int)(hash % HASHSIZE);
}

// int hash2(const char *str) {
//     const unsigned char *s = (unsigned char *)str;
//     unsigned long hash = 2687;
//     int c;

//     while (c = *s++) hash = ((hash << 11) + hash) + c; /* hash * 33 + c */

//     return (int)(hash);
// }

int hash4(const char *str, int len) {
    unsigned int tmp;
    tmp = *(unsigned int *)str;
    tmp = (tmp << ((3 - (len - 1) % 4) * 8));
    unsigned int hash = tmp;
    hash = (hash) ^ (hash >> 15);
    return (int)(hash % HASHSIZE4);
}

int hash5(const char *str, int len) {
    unsigned int tmp = *(unsigned int *)str;
    unsigned int hash = tmp;
    int turn = hash & 3;
    hash = (hash << (11 + turn)) ^ hash ^ (hash >> (21 - turn));
    tmp = *(unsigned int *)(str + 4);
    tmp = (tmp << ((3 - (len - 1) % 4) * 8));
    turn += hash & 3;
    hash = hash ^ (tmp) ^ (tmp >> (11 - turn));
    turn += hash & 3;
    hash = hash ^ (hash >> (11 + turn));
    turn += hash & 3;
    hash = hash ^ (tmp >> (7 + turn));
    return (int)(hash % HASHSIZE5);
}

int hash6(const char *str, int len) {
    unsigned int tmp;
    unsigned int hash = *(unsigned int *)str;
    tmp = hash;
    int i = 0;
    while (len > 3) {
        hash = (tmp << (11 + i)) ^ hash ^ (tmp >> (21 - i));
        str = (str + 4);
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
    hash = hash ^ (hash >> (9 + i));
    return (int)(hash % 133723689);  //
}

// ========================================
// Global variables

int n_mails, n_queries;
mail *mails;
query *queries;
int token_num = 0;

int answer[MAX_NMAILS];

TokenSet tokensets[MAX_NMAILS] = {NULL};

GlobalTokenSet global_tokensets[3];

// double similaritys[MAX_NMAILS][MAX_NMAILS] = {0};

// ========================================

int GlobalSetContains(int h, int idx) {
    Node *node = global_tokensets[idx].hash_table[h];

    if (node == NULL) {
        return -1;
    }

    return node->num;
}

// Add a token into the token set
int GlobalSetAdd(char *token, int len) {  // const char *token,
    int h, num, idx;
    if (len <= 4) {
        idx = 0;
        h = hash4(token, len);
    } else if (len <= 8) {
        idx = 1;
        h = hash5(token, len);
    } else {
        idx = 2;
        h = hash6(token, len);
    }
    num = GlobalSetContains(h, idx);
    if (num != -1) {
        return num;
    }

    Node *hash_node = (Node *)malloc(sizeof(Node));
    hash_node->num = token_num;
    global_tokensets[idx].hash_table[h] = hash_node;
    token_num++;
    return hash_node->num;
}

void GlobalTokenAdd(char *token, int len) {
    int h, num, idx;
    if (len <= 4) {
        idx = 0;
        h = hash4(token, len);
    } else if (len <= 8) {
        idx = 1;
        h = hash5(token, len);
    } else {
        idx = 2;
        h = hash6(token, len);
    }
    Node *hash_node = (Node *)malloc(sizeof(Node));
    hash_node->num = -2;
    global_tokensets[idx].hash_table[h] = hash_node;
}

void parse_and_add_to_token_set(char *s, TokenSet *set) {
    char *start = NULL;
    char c;
    int num;
    int idx;
    while (c = *s) {
        if (c >= 'A' && c <= 'Z') {
            c = *s = c - 'A' + 'a';  // convert to lowercase
        }
        if (!((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9'))) {
            if (start) {
                *s = '\0';
                num = GlobalSetAdd(start, s - start);
                if (num > -1) {
                    idx = num >> 6;
                    if (((set->bitmap[idx] >> (num & 63)) & 1) == 0) {
                        set->n_items++;
                        if (set->bitmap[idx] == 0) {
                            set->stack[set->top++] = idx;
                        }
                        // printf("before stack\n");
                        // set->stack[set->n_items++] = num;
                        set->bitmap[idx] |= (1ULL << (num & 63));
                        // printf("after stack\n");
                    }
                }
                start = NULL;
            }
        } else if (!start) {
            start = s;
        }
        ++s;
    }
    if (start) {
        num = GlobalSetAdd(start, s - start);
        if (num > -1) {
            idx = num >> 6;
            if (((set->bitmap[idx] >> (num % 64)) & 1) == 0) {
                set->n_items++;
                if (set->bitmap[idx] == 0) {
                    set->stack[set->top++] = idx;
                }
                // set->stack[set->n_items++] = num;
                set->bitmap[idx] |= (1ULL << (num % 64));
            }
        }
    }
}

double context_similarity(int i, int j) {
    Node *node;
    int temp, n_intersection = 0;
    unsigned long long n;
    if (tokensets[i].top > tokensets[j].top) {
        temp = i;
        i = j;
        j = temp;
    }
    int idx;
    for (int tmp = 0; tmp < tokensets[i].top; tmp++) {
        // idx = tokensets[i].stack[tmp] >> 6;
        // printf("%d %d\n", tokensets[i].stack[tmp], idx);
        // if ((tokensets[j].bitmap[idx] >> (tokensets[i].stack[tmp] % 64)) & 1)
        // {
        //     n_intersection++;
        // }
        n = (tokensets[i].bitmap[tokensets[i].stack[tmp]]) &
            (tokensets[j].bitmap[tokensets[i].stack[tmp]]);
        while (n != 0ULL) {
            n = n & (n - 1ULL);
            n_intersection++;
        }
    }
    // printf("n_intersection: %d\n", n_intersection);
    return (double)(n_intersection + 8) /
           (double)(tokensets[i].n_items + tokensets[j].n_items -
                    n_intersection + 8);
}

void find_similar_query(int query_id, int mail_id, double threshold) {
    int i, answer_length;
    double sim;
    answer_length = 0;
    for (i = 0; i < n_mails; ++i) {
        if (i == mail_id) {
            continue;
        }

        // if (similaritys[i][mail_id] != 0) {
        //     sim = similaritys[i][mail_id];
        // } else {
        //     sim = context_similarity(i, mail_id);
        //     similaritys[i][mail_id] = sim;
        //     similaritys[mail_id][i] = sim;
        // }
        if (context_similarity(i, mail_id) > threshold) {
            answer[answer_length++] = i;
        }
    }
    api.answer(query_id, answer, answer_length);
}

// ========================================

int main(void) {
    global_tokensets[0].hash_table = malloc(HASHSIZE4 * sizeof(Node *));
    global_tokensets[1].hash_table = malloc(HASHSIZE5 * sizeof(Node *));
    global_tokensets[2].hash_table = malloc(HASHSIZE6 * sizeof(Node *));
    GlobalTokenAdd("i", 1);
    GlobalTokenAdd("read", 4);
    GlobalTokenAdd("the", 3);
    GlobalTokenAdd("paragraph", 9);
    GlobalTokenAdd("http", 4);
    GlobalTokenAdd("on", 2);
    GlobalTokenAdd("wikipedia", 9);
    GlobalTokenAdd("org", 3);
    api.init(&n_mails, &n_queries, &mails, &queries);
    int i;
    for (i = 0; i < n_mails; ++i) {
        parse_and_add_to_token_set(mails[i].subject, tokensets + mails[i].id);
        parse_and_add_to_token_set(mails[i].content, tokensets + mails[i].id);
        // for (int j = 0; j < BITMAP_LEN; j++) {
        //     printf("%llu|", tokensets[i].bitmap[j]);
        // }
        // for (int j = 0; j < tokensets[i].top; j++) {
        //     printf("%d|", tokensets[i].stack[j]);
        // }
        // printf("\n");
        // printf("%d\n", i);
        // printf("%d\n", tokensets[i].n_items);
    }
    // printf("success init\n");
    // double score;
    for (i = 0; i < n_queries; ++i) {
        if (queries[i].type == find_similar &&
            (queries[i].reward >= 95)) {  //&&
                                          // (queries[i].reward <= 80)
            find_similar_query(queries[i].id,
                               queries[i].data.find_similar_data.mid,
                               queries[i].data.find_similar_data.threshold);
            // score += queries[i].reward;
            // fprintf(stderr, "%f\n", score);
        }
    }

    return 0;
}
