#include "api.h"
#define HASHSIZE 19697
#define INDEXSIZE 1000
#define MAX_NMAILS 10000

// ========================================
// Token set implementation

typedef struct Node {
    struct Node *next;
    const char *s;
} Node;

typedef struct {
    Node *hash_table[HASHSIZE];
    Node *items;
    int n_items;
} TokenSet;

// Hash function for string
// Reference:
// https://stackoverflow.com/questions/7666509/hash-function-for-string
unsigned long hash(const char *str) {
    const unsigned char *s = (unsigned char *)str;
    unsigned long hash = 5381;
    int c;

    while (c = *s++) hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

    return hash % HASHSIZE;
}

// See whether token in the token set or not
bool SetContains(TokenSet *set, const char *token) {
    if (!set) return false;
    unsigned long h = hash(token);
    Node *node = set->hash_table[h];
    while (node != NULL) {
        if (!strcmp(token, node->s)) return true;
        node = node->next;
    }
    return false;
}

// Add a token into the token set
void SetAdd(TokenSet *set, const char *token) {
    if (SetContains(set, token)) return;
    unsigned long h = hash(token);
    Node *hash_node = (Node *)malloc(sizeof(Node));
    Node *item_node = (Node *)malloc(sizeof(Node));

    hash_node->s = token;
    hash_node->next = set->hash_table[h];
    set->hash_table[h] = hash_node;

    item_node->s = token;
    item_node->next = set->items;
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

TokenSet global_tokenset;
double similaritys[MAX_NMAILS][MAX_NMAILS] = {0};

// ========================================

bool GlobalSetContains(int h, int h2) {
    Node *node = global_tokenset.hash_table[h];
    if (node != NULL && node->hash2 == h2) {
        return true;
    }
    return false;
}

void parse_and_add_to_global_set(char *s) {
    char *start = NULL;
    char c;
    unsigned long h = 5381;
    unsigned long h2 = 2687;
    int cnt;
    while (c = *s) {
        if (c >= 'A' && c <= 'Z') {
            c = *s = c - 'A' + 'a';  // convert to lowercase
        }
        if (!((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9'))) {
            if (start) {
                *s = '\0';
                h = h % HASHSIZE;
                h2 = h2 % HASHSIZE;
                SetAdd(&global_tokenset, h, h2);
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
        SetAdd(&global_tokenset, (h % HASHSIZE), (h2 % HASHSIZE));  // start,
}

void parse_and_add_to_token_set(char *s, TokenSet *set) {
    char *start = NULL;
    char c;
<<<<<<< HEAD
    unsigned long h = 5381;
    unsigned long h2 = 2687;
    int cnt;
    == == == =
>>>>>>> 21dd5956abac4b506d66eb7232e357753627d906
                 while (c = *s) {
        if (c >= 'A' && c <= 'Z')
            c = *s = c - 'A' + 'a';  // convert to lowercase
        if (!((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9'))) {
            if (start) {
                *s = '\0';
<<<<<<< HEAD
                h = h % HASHSIZE;
                h2 = h2 % HASHSIZE;
                if (GlobalSetContains(h, h2)) {
                    cnt++;
                    if (cnt > 3) {
                        start = NULL;
                        break;
                    }
                } else if (!SetContains(set, h, h2)) {
                    SetAdd(set, h, h2);
                    cnt = 0;
                } else {
                    cnt = 0;
                }
                // start,
                == == == = SetAdd(set, start);
>>>>>>> 21dd5956abac4b506d66eb7232e357753627d906
                start = NULL;
            }
        } else if (!start) {
            start = s;
        }
        ++s;
    }
    if (start) SetAdd(set, start);
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
        if (SetContains(tokensets + j, node->s)) ++n_intersection;
        node = node->next;
    }
    // n_intersection += global_tokenset.n_items;
    return (double)(n_intersection + global_tokenset.n_items) /
           (tokensets[i].n_items + tokensets[j].n_items - n_intersection +
            global_tokenset.n_items);
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
    clock_t start;
    start = clock();
    api.init(&n_mails, &n_queries, &mails, &queries);
    // printf("%f\n", (float)(clock() - start) / CLOCKS_PER_SEC);
    int i;
    char common[50] = "I read the paragraph on http://wikipedia.org ";
    global_tokenset.hash_table = malloc(HASHSIZE * sizeof(Node *));
    parse_and_add_to_global_set(common);
    for (i = 0; i < n_mails; ++i) {
        parse_and_add_to_token_set(mails[i].subject, tokensets + mails[i].id);
        parse_and_add_to_token_set(mails[i].content, tokensets + mails[i].id);
    }
    // printf("%f\n", (float)(clock() - start) / CLOCKS_PER_SEC);
    clock_t old_start = clock();
    double score = 0;
    for (i = 0; i < n_queries; ++i) {
        if (queries[i].type == find_similar) {
            find_similar_query(queries[i].id,
                               queries[i].data.find_similar_data.mid,
                               queries[i].data.find_similar_data.threshold);
            /*score += queries[i].reward;*/
            /*fprintf(stderr, "%f\n", score);*/

            // printf("%f\n", (float)(clock() - old_start) / CLOCKS_PER_SEC);
            // printf("%f\n", (float)(clock() - start) / CLOCKS_PER_SEC);
            old_start = clock();
        }
    }

    return 0;
}
