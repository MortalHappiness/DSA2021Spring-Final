#include "api.h"
#define HASHSIZE 10000000
#define MAX_NMAILS 10000
#define BLOOM_K 8

typedef struct Node {
    struct Node *next;
    int len;
    unsigned int hash[BLOOM_K];
    const char *s;
} Node;

typedef struct {
    // int bits;
    Node *head;
    Node *tail;
    unsigned char *hashtable;
    int n_items;
} TokenSet;

unsigned int murmurhash3(char *key, int len, const unsigned int seed) {
    unsigned int c1 = 0xcc9e2d51;
    unsigned int c2 = 0x1b873593;
    int r1 = 15;
    int r2 = 13;
    int m = 5;
    unsigned int n = 0xe6546b64;
    unsigned int hash = seed;
    unsigned int k;
    // int s = 0;
    int len_copy = len;
    while (len >= 4) {
        k = (key[len - 4] << 24) | (key[len - 3] << 16) | (key[len - 2] << 8) |
            (key[len - 1]);
        k = k * c1;
        k = (k << r1) | (k >> (32 - r1));
        k = k * c2;
        hash = hash ^ k;
        k = (k << r2) | (k >> (32 - r2));
        hash = hash * m + n;
        len -= 4;
    }
    k = 0;
    switch (len) {
        case 3:
            k |= key[2];
        case 2:
            k |= key[1] << 8;
        case 1:
            k |= key[0] << 16;
    }
    k = k * c1;
    k = (k << r1) | (k >> (32 - r1));
    k = k * c2;
    hash = hash ^ k;
    hash = hash ^ len_copy;
    hash = hash ^ (hash >> 16);
    hash = hash ^ 0x85ebca6b;
    hash = hash ^ (hash >> 13);
    hash = hash ^ 0xc2b2ae35;
    hash = hash ^ (hash >> 16);
    return hash;
}
void bloom_add_token(TokenSet *set, char *token, int len) {
    Node *hash_node = (Node *)malloc(sizeof(Node));
    hash_node->s = token;
    hash_node->next = NULL;
    hash_node->len = len;
    if (set->head == NULL) {
        set->head = hash_node;
        set->tail = hash_node;
    } else {
        set->tail->next = hash_node;
        set->tail = hash_node;
    }
    for (int i = 0; i < BLOOM_K; i++) {
        int hash = murmurhash3(token, len, i) % HASHSIZE;
        int offset = hash >> 3;
        set->hashtable[offset] |= 1 << (hash % 8);
        hash_node->hash[i] = hash;
    }
}

bool bloom_check_token(TokenSet *set, Node *token_node) {
    for (int i = 0; i < BLOOM_K; i++) {
        // unsigned int hash = murmurhash3(token, len, i) % HASHSIZE;
        unsigned int hash = token_node->hash[i];
        int offset = hash >> 3;
        if (!(set->hashtable[offset] & (1 << (hash % 8)))) {
            return false;
        }
    }
    return true;
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
    while (c = *s) {
        if (c >= 'A' && c <= 'Z')
            c = *s = c - 'A' + 'a';  // convert to lowercase
        if (!((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9'))) {
            if (start) {
                *s = '\0';
                bloom_add_token(set, start, s - start);
                start = NULL;
            }
        } else if (!start) {
            start = s;
        }
        ++s;
    }
    if (start) bloom_add_token(set, start, s - start);
}

double context_similarity(int i, int j) {
    Node *node;
    int temp, n_intersection;

    if (tokensets[i].n_items > tokensets[j].n_items) {
        temp = i;
        i = j;
        j = temp;
    }
    node = tokensets[i].head;
    n_intersection = 0;
    while (node) {
        if (bloom_check_token(tokensets + j, node)) {
            ++n_intersection;
        }
        node = node->next;
    }
    return (double)n_intersection /
           (tokensets[i].n_items + tokensets[j].n_items - n_intersection);
}

void find_similar_query(int query_id, int mail_id, double threshold) {
    int i, answer_length;

    answer_length = 0;
    for (i = 0; i < n_mails; ++i) {
        if (i == mail_id) continue;
        if (context_similarity(i, mail_id) > threshold)
            answer[answer_length++] = i;
    }
    api.answer(query_id, answer, answer_length);
}

// ========================================

int main(void) {
    api.init(&n_mails, &n_queries, &mails, &queries);

    int i;

    for (i = 0; i < n_mails; ++i) {
        tokensets[i].hashtable = malloc((HASHSIZE >> 3) * sizeof(char));
        parse_and_add_to_token_set(mails[i].subject, tokensets + mails[i].id);
        parse_and_add_to_token_set(mails[i].content, tokensets + mails[i].id);
    }

    double score = 0;
    for (i = 0; i < n_queries; ++i) {
        if (queries[i].type == find_similar) {
            find_similar_query(queries[i].id,
                               queries[i].data.find_similar_data.mid,
                               queries[i].data.find_similar_data.threshold);
            /*score += queries[i].reward;*/
            /*fprintf(stderr, "%f\n", score);*/
        }
    }

    return 0;
}