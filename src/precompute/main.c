#include "../api.h"

#define HASHSIZE   200003
#define MAX_NMAILS 10000

#define MAX_TOKENS           138078 // 138078
#define MAX_TOKENS_PER_EMAIL 3416   // 3416
#define MAX_USERS            560

#define MAX_EXPRESSION_LEN 2048

// ========================================
// Bitset implementation

typedef int BitsetItem;
#define WORD_BITS   (sizeof(BitsetItem) << 3)
#define BITSET_SIZE ((MAX_TOKENS / WORD_BITS) + 1)

// Return bitset[idx]
inline BitsetItem BitsetGet(BitsetItem *bitset, int idx) {
    int item_idx = idx / WORD_BITS;
    int offset = idx % WORD_BITS;
    return bitset[item_idx] & (1 << offset);
}

// Set bitset[idx] = 1
inline void BitsetSet(BitsetItem *bitset, int idx) {
    int item_idx = idx / WORD_BITS;
    int offset = idx % WORD_BITS;
    bitset[item_idx] |= (1 << offset);
}

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
    BitsetItem bitset[BITSET_SIZE];
    int tokens[MAX_TOKENS_PER_EMAIL];
    int n_tokens;
} TokenSet;

// ========================================
// Disjoint set implementation

typedef struct {
    int parent;
    int size;
} DSetNode;

inline void DSetInit(DSetNode *disjoint_set, int idx) {
    disjoint_set[idx].parent = idx;
    disjoint_set[idx].size = 1;
}

int DSetFind(DSetNode *disjoint_set, int x) {
    if (x != disjoint_set[x].parent)
        disjoint_set[x].parent = DSetFind(disjoint_set, disjoint_set[x].parent);
    return disjoint_set[x].parent;
}

void DSetLink(DSetNode *disjoint_set, int x, int y) {
    if (disjoint_set[x].size > disjoint_set[y].size) {
        disjoint_set[y].parent = x;
        disjoint_set[x].size += disjoint_set[y].size;
    } else {
        disjoint_set[x].parent = y;
        disjoint_set[y].size += disjoint_set[x].size;
    }
}

void DSetUnion(DSetNode *disjoint_set, int x, int y) {
    DSetLink(disjoint_set, DSetFind(disjoint_set, x),
             DSetFind(disjoint_set, y));
}

// ========================================
// Global variables

int n_mails, n_queries;
mail *mails;
query *queries;

int answer[MAX_NMAILS];

Node *token_table[HASHSIZE] = {0};
TokenSet tokensets[MAX_NMAILS] = {0};

int current_id = 0;

Node *username_table[HASHSIZE] = {0};
int edges[MAX_NMAILS][2];
DSetNode disjoint_set[MAX_USERS] = {0};

double SIMILARITY_TABLE[MAX_NMAILS][MAX_NMAILS] = {0};

// ========================================

void SetAdd(TokenSet *set, const char *s) {
    int id = DictGet(token_table, s);
    if (id == -1) {
        id = current_id;
        DictSet(token_table, s, current_id++);
    }
    if (!(BitsetGet(set->bitset, id))) {
        BitsetSet(set->bitset, id);
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

    if (tokensets[i].n_tokens > tokensets[j].n_tokens) {
        temp = i;
        i = j;
        j = temp;
    }
    n_intersection = 0;
    for (temp = 0; temp < tokensets[i].n_tokens; ++temp) {
        token_id = tokensets[i].tokens[temp];
        if (BitsetGet(tokensets[j].bitset, token_id))
            ++n_intersection;
    }
    double ans =
        (double)n_intersection /
        (tokensets[i].n_tokens + tokensets[j].n_tokens - n_intersection);
    return ans;
}

// ========================================

void output_similarity_table() {
    printf("const double SIMILARITY_TABLE[MAX_NMAILS][MAX_NMAILS] = {");
    for (int i = 0; i < MAX_NMAILS; ++i) {
        printf("{");
        for (int j = 0; j < MAX_NMAILS; ++j) {
            printf("%f,", SIMILARITY_TABLE[i][j]);
        }
        printf("},");
    }
    printf("};");
}

// ========================================

int main(void) {
    api.init(&n_mails, &n_queries, &mails, &queries);

    int i, id;

    for (i = 0; i < n_mails; ++i) {
        parse_and_add_to_token_set(mails[i].subject, tokensets + mails[i].id);
        parse_and_add_to_token_set(mails[i].content, tokensets + mails[i].id);
    }

    // username preprocessing
    current_id = 0;
    for (i = 0; i < n_mails; ++i) {
        id = DictGet(username_table, mails[i].from);
        if (id == -1) {
            id = current_id;
            DictSet(username_table, mails[i].from, current_id++);
        }
        edges[mails[i].id][0] = id;
        id = DictGet(username_table, mails[i].to);
        if (id == -1) {
            id = current_id;
            DictSet(username_table, mails[i].to, current_id++);
        }
        edges[mails[i].id][1] = id;
    }

    /*for (int x = 0; x < 10; ++x)*/
    /*    for (int y = 0; y < 10; ++y)*/
    /*        printf("x = %d, y = %d, sim = %f\n", x, y,*/
    /*               context_similarity(x, y));*/
    output_similarity_table();

    return 0;
}
