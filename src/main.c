#include "api.h"
#define HASHSIZE 10461365
#define MAX_NMAILS 10000
#define TOKENSIZE 138078
#define BITMAP_LEN 4315
// ========================================
// Token set implementation

typedef struct Node {
    struct Node *next;
    // int hash;
    // int hash2;
    int num;
    char *s;
} Node;

typedef struct {
    Node **hash_table;
    // Node *items;
    int n_items;
} GlobalTokenSet;

typedef struct {
    int n_items;
    unsigned int bitmap[BITMAP_LEN];
    int top;
    int stack[BITMAP_LEN];
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
    } /* hash * 33 + c */

    return (int)(hash % HASHSIZE);
}

// ========================================
// Global variables

int n_mails, n_queries;
mail *mails;
query *queries;
int token_num = 0;

int answer[MAX_NMAILS];

TokenSet tokensets[MAX_NMAILS] = {NULL};

GlobalTokenSet global_tokenset;

double similaritys[MAX_NMAILS][MAX_NMAILS] = {0};

// ========================================

int GlobalSetContains(int h, char*token) {
    Node *node = global_tokenset.hash_table[h];

    while (node != NULL) {
        if (!strcmp(token, node->s)) {
            return node->num;
        }
        node = node->next;
    }

    return -1;
}

// Add a token into the token set
int GlobalSetAdd(char *token) {  // const char *token,
    int h = hash(token);

    int num = GlobalSetContains(h, token);
    if (num != -1) {
        return num;
    }

    Node *hash_node = (Node *)malloc(sizeof(Node));
    Node *item_node = (Node *)malloc(sizeof(Node));

    hash_node->next = global_tokenset.hash_table[h];
    hash_node->num = token_num;
    hash_node->s = token;
    global_tokenset.hash_table[h] = hash_node;

    ++(global_tokenset.n_items);
    token_num++;
    return hash_node->num;
}

void parse_and_add_to_token_set(char *s, TokenSet *set) {
    char *start = NULL;
    char c;
    unsigned int num;
    int idx;
    while (c = *s) {
        if (c >= 'A' && c <= 'Z') {
            c = *s = c - 'A' + 'a';  // convert to lowercase
        }
        if (!((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9'))) {
            if (start) {
                *s = '\0';
                num = GlobalSetAdd(start);
                idx = num >> 5;
                if (((set->bitmap[idx] >> (num % 32)) & 1) == 0) {
                    set->n_items++;
                    if (set->bitmap[idx] == 0) {
                        set->stack[set->top++] = idx;
                    }
                    set->bitmap[idx] |= (1 << (num % 32));
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
        idx = num >> 5;
        if (((set->bitmap[idx] >> (num % 32)) & 1) == 0) {
            set->n_items++;
            if (set->bitmap[idx] == 0) {
                set->stack[set->top++] = idx;
            }
            set->bitmap[idx] |= 1 << (num % 32);
        }
    }
}

double context_similarity(int i, int j) {
    Node *node;
    int temp, n_intersection = 0;
    unsigned int n;
    if (tokensets[i].top > tokensets[j].top) {
        temp = i;
        i = j;
        j = temp;
    }

    for (int tmp = 0; tmp < tokensets[i].top; tmp++) {
        n = (tokensets[i].bitmap[tokensets[i].stack[tmp]]) &
            (tokensets[j].bitmap[tokensets[i].stack[tmp]]);
        while (n != 0) {
            n = n & (n - 1);
            n_intersection++;
        }
    }

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
    // clock_t start = clock();
    int i;
    global_tokenset.hash_table = malloc(HASHSIZE * sizeof(Node *));
    for (i = 0; i < n_mails; ++i) {
        parse_and_add_to_token_set(mails[i].subject, tokensets + mails[i].id);
        parse_and_add_to_token_set(mails[i].content, tokensets + mails[i].id);
    }

    for (i = 0; i < n_queries; ++i) {
        if (queries[i].type == find_similar && queries[i].reward >= 100) {  //
            find_similar_query(queries[i].id,
                               queries[i].data.find_similar_data.mid,
                               queries[i].data.find_similar_data.threshold);
            // score += queries[i].reward;
            // fprintf(stderr, "%f\n", score);
        }
    }

    return 0;
}
