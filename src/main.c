#include "api.h"
#define HASHSIZE   19697
#define INDEXSIZE  1000
#define MAX_NMAILS 10000

// ========================================
// Token set implementation

typedef struct Node {
    struct Node *next;
    int hash;
    int hash2;
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

    while (c = *s++)
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

    return (int)(hash % HASHSIZE);
}

int hash2(const char *str) {
    const unsigned char *s = (unsigned char *)str;
    unsigned long hash = 2687;
    int c;

    while (c = *s++)
        hash = ((hash << 11) + hash) + c; /* hash * 33 + c */

    return (int)(hash % HASHSIZE);
}

// See whether token in the token set or not
bool SetContains(TokenSet *set, int h, int h2) { // const char *token,
    Node *node = set->hash_table[h];
    while (node != NULL) {
        if (node->hash2 == h2)
            return true;
        node = node->next;
    }
    return false;
}

// Add a token into the token set
void SetAdd(TokenSet *set, int h, int h2) { // const char *token,
    if (SetContains(set, h, h2))
        return; // token,
    Node *hash_node = (Node *)malloc(sizeof(Node));
    Node *item_node = (Node *)malloc(sizeof(Node));

    hash_node->next = set->hash_table[h];
    hash_node->hash = h;
    hash_node->hash2 = h2;
    set->hash_table[h] = hash_node;

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
    char *start = NULL;
    char c;
    unsigned long h = 5381;
    unsigned long h2 = 2687;
    while (c = *s) {
        if (c >= 'A' && c <= 'Z') {
            c = *s = c - 'A' + 'a'; // convert to lowercase
        }
        if (!((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9'))) {
            if (start) {
                *s = '\0';
                SetAdd(set, (h % HASHSIZE),
                       (h2 % HASHSIZE)); // start,
                start = NULL;
                h = 5381;
                h2 = 2687;
            }
        } else if (!start) {
            start = s;
            h = ((h << 5) + h) + c;
            h2 = ((h2 << 11) + h2) + c;
        } else {
            h = ((h << 5) + h) + c;
            h2 = ((h2 << 11) + h2) + c;
        }
        ++s;
    }
    if (start)
        SetAdd(set, (h % HASHSIZE), (h2 % HASHSIZE)); // start,
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
        if (SetContains(tokensets + j, node->hash, node->hash2)) // node->s,
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
        if (i == mail_id)
            continue;
        if (similaritys[i][mail_id] != -1) {
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

    int i;

    for (i = 0; i < n_mails; ++i) {
        for (int j = 0; j < n_mails; ++j) {
            similaritys[i][j] = -1;
        }
        tokensets[i].hash_table = malloc(HASHSIZE * sizeof(Node *));
        parse_and_add_to_token_set(mails[i].subject, tokensets + mails[i].id);
        parse_and_add_to_token_set(mails[i].content, tokensets + mails[i].id);
    }

    /* double score = 0; */
    for (i = 0; i < n_queries; ++i) {
        if (queries[i].type == find_similar && queries[i].reward >= 80) {
            find_similar_query(queries[i].id,
                               queries[i].data.find_similar_data.mid,
                               queries[i].data.find_similar_data.threshold);
            /* score += queries[i].reward;
            fprintf(stderr, "%f\n", score);*/
        }
    }

    return 0;
}
