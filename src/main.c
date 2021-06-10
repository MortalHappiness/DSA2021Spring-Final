#include "api.h"

#define HASHSIZE   19697
#define INDEXSIZE  1000
#define MAX_NMAILS 10000

#define MAX_TOKENS           140000 // 138078
#define MAX_TOKENS_PER_EMAIL 4000   // 3416

// ========================================
// Dictionary implementation (key: string, value: non-negative int)

typedef struct Node {
    struct Node *next;
    const char *key;
    int value;
} Node;

// Hash function for string
// Reference:
// https://stackoverflow.com/questions/7666509/hash-function-for-string
unsigned long hash(const char *str) {
    const unsigned char *s = (unsigned char *)str;
    unsigned long hash = 5381;
    int c;

    while (c = *s++)
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

    return hash % HASHSIZE;
}

Node *_DictGetNode(Node **table, const char *key, unsigned long h) {
    Node *node = table[h];
    while (node != NULL) {
        if (!strcmp(key, node->key))
            return node;
        node = node->next;
    }
    return NULL;
}

// Return table[key] if key in table else -1
int DictGet(Node **table, const char *key) {
    unsigned long h = hash(key);
    Node *node = _DictGetNode(table, key, h);
    if (node != NULL)
        return node->value;
    return -1;
}

// Set table[key] = value
void DictSet(Node **table, const char *key, int value) {
    unsigned long h = hash(key);
    Node *node = _DictGetNode(table, key, h);
    if (node != NULL) {
        node->value = value;
        return;
    }
    node = (Node *)malloc(sizeof(Node));
    node->key = key;
    node->value = value;
    node->next = table[h];
    table[h] = node;
}

// ========================================
// Token set implementation

typedef struct {
    bool set[MAX_TOKENS];
    int tokens[MAX_TOKENS_PER_EMAIL];
    int n_tokens;
} TokenSet;

// ========================================
// Global variables

int n_mails, n_queries;
mail *mails;
query *queries;

int answer[MAX_NMAILS];

Node *token_table[HASHSIZE] = {0};
TokenSet tokensets[MAX_NMAILS] = {0};

int current_id = 0;

/*double similarity_table[MAX_NMAILS][MAX_NMAILS] = {0};*/

// ========================================

void SetAdd(TokenSet *set, const char *s) {
    int id = DictGet(token_table, s);
    if (id == -1) {
        id = current_id;
        DictSet(token_table, s, current_id++);
    }
    if (!(set->set[id])) {
        set->set[id] = true;
        set->tokens[(set->n_tokens)++] = id;
    }
}

void parse_and_add_to_token_set(char *s, TokenSet *set) {
    char *start = NULL;
    char c;
    while (c = *s) {
        if (c >= 'A' && c <= 'Z')
            c = *s = c - 'A' + 'a'; // convert to lowercase
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
    if (start)
        SetAdd(set, start);
}

double context_similarity(int i, int j) {
    int temp, n_intersection, token_id;

    /*if (similarity_table[i][j] != 0)*/
    /*    return similarity_table[i][j];*/

    if (tokensets[i].n_tokens > tokensets[j].n_tokens) {
        temp = i;
        i = j;
        j = temp;
    }
    n_intersection = 0;
    for (temp = 0; temp < tokensets[i].n_tokens; ++temp) {
        token_id = tokensets[i].tokens[temp];
        if (tokensets[j].set[token_id])
            ++n_intersection;
    }
    double ans =
        (double)n_intersection /
        (tokensets[i].n_tokens + tokensets[j].n_tokens - n_intersection);
    /*if (ans == 0)*/
    /*    ans = -1;*/
    /*similarity_table[i][j] = ans;*/
    /*similarity_table[j][i] = ans;*/
    return ans;
}

void find_similar_query(int query_id, int mail_id, double threshold) {
    int i, answer_length;

    answer_length = 0;
    for (i = 0; i < n_mails; ++i) {
        if (i == mail_id)
            continue;
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
