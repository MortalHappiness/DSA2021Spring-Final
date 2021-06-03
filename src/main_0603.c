#include "api.h"
#define HASHSIZE 19697
#define INDEXSIZE 1000
#define MAX_NMAILS 10000

// ========================================
// Token set implementation

typedef struct Node {
    struct Node *next;
    int hash;
    int hash2;
    const char *s;
} Node;

typedef struct {
    Node **hash_table;
    Node *items;
    int n_items;
} TokenSet;

double similaritys[MAX_NMAILS][MAX_NMAILS];
// Hash function for string
// Reference:
// https://stackoverflow.com/questions/7666509/hash-function-for-string
int hash(const char *str) {
    const unsigned char *s = (unsigned char *)str;
    unsigned long hash = 5381;
    int c;

    while (c = *s++) hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

    return (int)(hash % HASHSIZE);
}

int hash2(const char *str) {
    const unsigned char *s = (unsigned char *)str;
    unsigned long hash = 2687;
    int c;

    while (c = *s++) hash = ((hash << 11) + hash) + c; /* hash * 33 + c */

    return (int)(hash % HASHSIZE);
}

// See whether token in the token set or not
bool SetContains(TokenSet *set, const char *token, int h, int h2) {
    if (!set) return false;
    // unsigned long h = hash(token);
    // int h
    // if (h == -1) {
    //     h = hash(token);
    // }

    Node *node = set->hash_table[h];
    while (node != NULL) {
        // if (!strcmp(token, node->s)) return true;
        if (node->hash2 == h2) return true;
        // printf("strcmp\n");
        node = node->next;
    }
    // printf("leave\n");
    return false;
}

// Add a token into the token set
void SetAdd(TokenSet *set, const char *token) {
    int h = hash(token);
    int h2 = hash2(token);
    if (SetContains(set, token, h, h2)) return;
    Node *hash_node = (Node *)malloc(sizeof(Node));
    Node *item_node = (Node *)malloc(sizeof(Node));

    hash_node->s = token;
    hash_node->next = set->hash_table[h];
    hash_node->hash = h;
    hash_node->hash2 = h2;
    set->hash_table[h] = hash_node;

    item_node->s = token;
    item_node->next = set->items;
    item_node->hash = h;
    item_node->hash2 = h2;
    set->items = item_node;
    ++(set->n_items);
}

// ========================================
// Global variables

int n_mails, n_queries;
mail *mails;
query *queries;

int answer[MAX_NMAILS];

TokenSet tokensets[MAX_NMAILS] = {NULL};

// ========================================

void parse_and_add_to_token_set(char *s, TokenSet *set) {
    // printf("start parse\n");
    char *start = NULL;
    char c;
    while (c = *s) {
        if (c >= 'A' && c <= 'Z')
            c = *s = c - 'A' + 'a';  // convert to lowercase
        if (!((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9'))) {
            if (start) {
                *s = '\0';
                SetAdd(set, start);
                start = NULL;
            }
        } else if (!start) {
            start = s;
        }
        ++s;
    }
    if (start) SetAdd(set, start);
    // printf("finish parse\n");
}

double context_similarity(int i, int j) {
    Node *node;
    int temp, n_intersection;

    if (tokensets[i].n_items > tokensets[j].n_items) {
        temp = i;
        i = j;
        j = temp;
    }
    node = tokensets[i].items;
    n_intersection = 0;
    while (node) {
        if (SetContains(tokensets + j, node->s, node->hash, node->hash2))
            ++n_intersection;
        node = node->next;
    }
    return (double)n_intersection /
           (tokensets[i].n_items + tokensets[j].n_items - n_intersection);
}

void find_similar_query(int query_id, int mail_id, double threshold) {
    int i, answer_length;
    double sim;
    answer_length = 0;
    for (i = 0; i < n_mails; ++i) {
        if (i == mail_id) continue;
        if (similaritys[i][mail_id] != -1) {
            sim = similaritys[i][mail_id];
            // printf("%f\n", sim);
        } else {
            // printf("here\n");
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
    // clock_t start;
    // start = clock();
    api.init(&n_mails, &n_queries, &mails, &queries);
    // printf("%f\n", (float)(clock() - start) / CLOCKS_PER_SEC);
    // printf("printf init\n");
    int i;

    for (i = 0; i < n_mails; ++i) {
        for (int j = 0; j < n_mails; ++j) {
            similaritys[i][j] = -1;
        }
        tokensets[i].hash_table = malloc(HASHSIZE * sizeof(Node *));
        parse_and_add_to_token_set(mails[i].subject, tokensets + mails[i].id);
        parse_and_add_to_token_set(mails[i].content, tokensets + mails[i].id);
    }
    // printf("%f\n", (float)(clock() - start) / CLOCKS_PER_SEC);
    clock_t old_start = clock();
    double score = 0;
    for (i = 0; i < n_queries; ++i) {
        if (queries[i].type == find_similar) {
            // printf("here\n");
            find_similar_query(queries[i].id,
                               queries[i].data.find_similar_data.mid,
                               queries[i].data.find_similar_data.threshold);
            /*score += queries[i].reward;*/
            /*fprintf(stderr, "%f\n", score);*/

            // printf("%f\n", (float)(clock() - old_start) / CLOCKS_PER_SEC);
            // printf("%f\n", (float)(clock() - start) / CLOCKS_PER_SEC);
            // old_start = clock();
        }
    }

    return 0;
}
