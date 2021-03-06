#include "api.h"

#define HASHSIZE 88349933
#define USERHASHSIZE 5309
#define MAX_NMAILS 10000

#define MAX_TOKENS 138078         // 138078
#define MAX_TOKENS_PER_EMAIL 3416 // 3416
#define MAX_USERS 560

#define MAX_EXPRESSION_LEN 2048

#define OP_NOT (-1)
#define OP_AND (-2)
#define OP_OR (-3)

// ========================================
// Bitset implementation

typedef int BitsetItem;
#define WORD_BITS (sizeof(BitsetItem) << 3)
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
// Token set implementation

typedef struct TokenSet {
    BitsetItem bitset[BITSET_SIZE];
    int tokens[MAX_TOKENS_PER_EMAIL];
    int n_tokens;
    int n_top;
} TokenSet;
// ========================================
// Dictionary implementation (key: string, value: non-negative int)

typedef struct Node {
    struct Node *next;
    // int set[3];
    struct TokenSet *top;
    // const char *key;
    int len;
    int value;
} Node;

// Hash function for string
// Reference:
// https://stackoverflow.com/questions/7666509/hash-function-for-string
int hash(const char *str) {
    const unsigned char *s = (unsigned char *)str;
    unsigned long hash = 44;
    int c;

    while (c = *s++)
        hash = ((hash << 7) + hash) + c; /* hash * 33 + c */

    return (int)(hash % HASHSIZE);
}

// int userhash(const char *str) {
//     const unsigned char *s = (unsigned char *)str;
//     unsigned long hash = 44;
//     int c, len = 0;

//     while (c = *s++) {
//         hash = ((hash << 7) + hash) + c; /* hash * 33 + c */
//         len++;
//     }
//     // printf("%s %d\n", str, len);
//     return (int)(hash % USERHASHSIZE);
// }

Node *_DictGetNode(Node **table, int len, int h) {
    Node *node = table[h];
    while (node != NULL) {
        if (len == node->len)
            return node;
        node = node->next;
    }
    return NULL;
}

// Return table[key] if key in table else -1
int DictGet(Node **table, int len, int h) {
    // int h = hash(key);
    Node *node = table[h];
    while (node != NULL) {
        if (len == node->len)
            return node->value;
        node = node->next;
    }
    return -1;
    // Node *node = _DictGetNode(table, len, h);
    // if (node != NULL)
    //     return node->value;
    // return -1;
}

// Set table[key] = value
void DictSet(Node **table, int len, int h, int value) {
    Node *node = (Node *)malloc(sizeof(Node));
    node->len = len;
    node->value = value;
    node->next = table[h];
    // node->first = set;
    table[h] = node;
}

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

int DSetLink(DSetNode *disjoint_set, int x, int y) {
    if (disjoint_set[x].size > disjoint_set[y].size) {
        disjoint_set[y].parent = x;
        disjoint_set[x].size += disjoint_set[y].size;
        return disjoint_set[x].size;
    } else {
        disjoint_set[x].parent = y;
        disjoint_set[y].size += disjoint_set[x].size;
        return disjoint_set[y].size;
    }
}

int DSetUnion(DSetNode *disjoint_set, int x, int y) {
    return DSetLink(disjoint_set, DSetFind(disjoint_set, x),
                    DSetFind(disjoint_set, y));
}

// ========================================
// Global variables

int n_mails, n_queries;
mail *mails;
query *queries;

int answer[MAX_NMAILS];

Node **token_table; //[HASHSIZE] = {0};
TokenSet tokensets[MAX_NMAILS] = {0};

int current_id = 0;

Node *username_table[USERHASHSIZE] = {0};
int edges[MAX_NMAILS][2];
DSetNode disjoint_set[MAX_USERS] = {0};

double similarity_table[MAX_NMAILS][MAX_NMAILS] = {0};
bool similarity_available[MAX_NMAILS] = {false};

// ========================================

// Return table[key] if key in table else -1

void SetAdd(TokenSet *set, const char *s, int len, int mail_id) {
    int h = hash(s);
    int id;
    Node *node = _DictGetNode(token_table, len, h);
    if (node != NULL) {
        id = node->value;
    } else {
        id = current_id;
        node = (Node *)malloc(sizeof(Node));
        node->len = len;
        node->value = id;
        node->next = token_table[h];
        node->top = set;
        token_table[h] = node;
        BitsetSet(set->bitset, id);
        (set->n_tokens)++;
        current_id++;
        return;
    }

    if (id == -2) {
        return;
    }

    if (!(BitsetGet(set->bitset, id))) {
        BitsetSet(set->bitset, id);
        if (node->top != -1) {
            // for (int j = 0; j < node->top; j++) {
            //     pre_intersections[node->set[j]][mail_id] += 1;
            // }
            // node->set[node->top++] = mail_id;
            // (node->set[2])->tokens[((node->set[2])->n_top)++] = id;
            // node->set[2] = -1;
            // BitsetSet(set->bitset, id);
            (node->top)->tokens[((node->top)->n_top)++] = id;
            node->top = -1;
        }
        // else if (node->top == 3) {
        //     pre_intersections[node->set[0]][node->set[1]] -= 1;
        //     pre_intersections[node->set[0]][node->set[2]] -= 1;
        //     pre_intersections[node->set[1]][node->set[2]] -= 1;
        //     for (int j = 0; j < 3; j++) {
        //         (tokensets + node->set[j])
        //             ->tokens[((tokensets + node->set[j])->n_top)++] = id;
        //     }
        //     node->top++;
        //     set->tokens[(set->n_top)++] = id;
        // } else {
        set->tokens[(set->n_top)++] = id;
        // }
        set->n_tokens++;
    }
}

int UserSetAdd(TokenSet *set, const char *s) {
    // const unsigned char *str = (unsigned char *)s;
    unsigned long hash = 44;
    int c, len = 0;

    while (c = *s++) {
        hash = ((hash << 7) + hash) + c; /* hash * 33 + c */
        len++;
    }
    // printf("%s %d\n", str, len);
    // return (int)(hash % USERHASHSIZE);
    int h = (int)(hash % USERHASHSIZE);
    // int len = strlen(s);
    int id = DictGet(username_table, len, h);
    if (id == -1) {
        id = current_id;
        DictSet(username_table, len, h, current_id++);
    }
    return id;
}

void parse_and_add_to_token_set(char *s, TokenSet *set, int mail_id) {
    char *start = NULL;
    char c;
    while (c = *s) {
        if (c >= 'A' && c <= 'Z')
            c = *s = c - 'A' + 'a'; // convert to lowercase
        if (!((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9'))) {
            if (start) {
                *s = '\0';
                // printf("before setadd\n");
                SetAdd(set, start, s - start, mail_id);
                // printf("after setadd\n");
                start = NULL;
            }
        } else if (!start) {
            start = s;
        }
        ++s;
    }
    if (start)
        SetAdd(set, start, s - start, mail_id);
}

double context_similarity(int i, int j) {
    int temp, n_intersection, token_id;
    // int bound = (double)((threshold * (double)(tokensets[i].n_tokens +
    //                                            tokensets[j].n_tokens +
    //                                            8)) -
    //                      (double)8) /
    //             (double)(1 + threshold);
    /*if (similarity_table[i][j] != 0)*/
    /*    return similarity_table[i][j];*/

    n_intersection = 0;
    if (tokensets[i].n_top > tokensets[j].n_top) {
        temp = i;
        i = j;
        j = temp;
    }
    for (temp = 0; temp < tokensets[i].n_top; ++temp) {
        token_id = tokensets[i].tokens[temp];
        if (BitsetGet(tokensets[j].bitset, token_id))
            ++n_intersection;
        // if (n_intersection > bound) {
        //     return true;
        // }
    }
    double ans =
        (double)(n_intersection + 8) /
        (tokensets[i].n_tokens + tokensets[j].n_tokens - n_intersection + 8);
    /*if (ans == 0)*/
    /*    ans = -1;*/
    /*similarity_table[i][j] = ans;*/
    /*similarity_table[j][i] = ans;*/
    return ans;
}

void build_similarity_table(int mail_id) {
    int i;
    for (i = 0; i < mail_id; ++i) {
        similarity_table[mail_id][i] = context_similarity(mail_id, i);
    }
    for (i = i + 1; i < n_mails; ++i) {
        similarity_table[mail_id][i] = context_similarity(mail_id, i);
    }
}

void find_similar_query(int query_id, int mail_id, double threshold) {
    int i, answer_length;
    double *row = similarity_table[mail_id];
    answer_length = 0;
    for (i = 0; i < mail_id; ++i) {
        if (row[i] > threshold) {
            answer[answer_length++] = i;
        }
        // if (context_similarity(i, mail_id, threshold) >
        //     threshold)  // > threshold
        //     answer[answer_length++] = i;
    }
    for (i = i + 1; i < n_mails; ++i) {
        // if (context_similarity(mail_id, i, threshold) >
        //     threshold)  // > threshold
        //     answer[answer_length++] = i;
        if (row[i] > threshold) {
            answer[answer_length++] = i;
        }
    }
    api.answer(query_id, answer, answer_length);
}

inline int _precedence(char c) {
    switch (c) {
    case '!':
        return 3;
    case '&':
        return 2;
    case '|':
        return 1;
    default:
        return 0;
    }
}

inline int _translate_to_op(char c) {
    switch (c) {
    case '!':
        return OP_NOT;
    case '&':
        return OP_AND;
    case '|':
        return OP_OR;
    default:
        return -100;
    }
}

bool expression_match_single(const int *postfix, int postfix_len, int id) {
    int i, token;
    bool stack[MAX_EXPRESSION_LEN];
    int stack_size = 0;
    for (i = 0; i < postfix_len; ++i) {
        token = postfix[i];
        if (token >= 0) {
            stack[stack_size++] = BitsetGet(tokensets[id].bitset, token);
        } else {
            switch (token) {
            case OP_NOT:
                stack[stack_size - 1] = !stack[stack_size - 1];
                break;
            case OP_AND:
                stack[stack_size - 2] &= stack[stack_size - 1];
                --stack_size;
                break;
            case OP_OR:
                stack[stack_size - 2] |= stack[stack_size - 1];
                --stack_size;
                break;
            default:
                break;
            }
        }
    }
    return stack[0];
}

void expression_match_query(int query_id, const char *expression) {
    int i, answer_length, buf_idx, id;
    char c, c1;
    unsigned long h;

    int postfix[MAX_EXPRESSION_LEN];
    int postfix_len = 0;
    char buf[MAX_EXPRESSION_LEN];
    char stack[MAX_EXPRESSION_LEN];
    int stack_size = 0;

    // Parse to postfix expression
    buf_idx = 0;
    for (i = 0; i <= strlen(expression); ++i) {
        c = expression[i];
        if (c >= 'A' && c <= 'Z')
            c = c - 'A' + 'a'; // convert to lowercase
        if ((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9')) {
            buf[buf_idx++] = c;
            continue;
        }
        if (buf_idx) {
            buf[buf_idx] = '\0';
            h = hash(buf);
            id = DictGet(token_table, buf, h);
            postfix[postfix_len++] = id;
            buf_idx = 0;
        }
        switch (c) {
        case '(':
            stack[stack_size++] = '(';
            break;
        case ')':
            while (stack_size--) {
                c1 = stack[stack_size];
                if (c1 == '(')
                    break;
                postfix[postfix_len++] = _translate_to_op(c1);
            }
            break;
        case '!':
        case '&':
        case '|':
            while (stack_size) {
                c1 = stack[stack_size - 1];
                if (_precedence(c1) < _precedence(c))
                    break;
                --stack_size;
                postfix[postfix_len++] = _translate_to_op(c1);
            }
            stack[stack_size++] = c;
            break;
        case '\0':
            while (stack_size--) {
                c1 = stack[stack_size];
                postfix[postfix_len++] = _translate_to_op(c1);
            }
            break;
        default:
            break;
        }
    }

    answer_length = 0;
    for (i = 0; i < MAX_NMAILS; ++i) {
        if (expression_match_single(postfix, postfix_len, i))
            answer[answer_length++] = i;
    }
    api.answer(query_id, answer, answer_length);
}

void group_analyse_query(int query_id, const int *mids, int len) {
    int i, id, user1, user2, n_groups, largest_group, idx, new_size;
    bool groups[MAX_USERS] = {0};

    for (i = 0; i < MAX_USERS; ++i)
        DSetInit(disjoint_set, i);

    largest_group = 1;
    for (i = 0; i < len; ++i) {
        id = mids[i];
        user1 = edges[id][0];
        user2 = edges[id][1];
        if (DSetFind(disjoint_set, user1) == DSetFind(disjoint_set, user2))
            continue;
        // TODO: return size in DSetUnion
        new_size = DSetUnion(disjoint_set, user1, user2);
        // idx = DSetFind(disjoint_set, user1);
        // new_size = disjoint_set[idx].size;
        if (new_size > largest_group)
            largest_group = new_size;
    }

    n_groups = 0;
    for (i = 0; i < MAX_USERS; ++i) {
        idx = DSetFind(disjoint_set, i);
        if (idx != i && !groups[idx]) {
            groups[idx] = true;
            ++n_groups;
        }
    }

    answer[0] = n_groups;
    answer[1] = largest_group;
    api.answer(query_id, answer, 2);
}

// ========================================

int main(void) {
    // username_table = malloc(USERHASHSIZE * sizeof(Node *));
    token_table = malloc(HASHSIZE * sizeof(Node *));
    api.init(&n_mails, &n_queries, &mails, &queries);
    // clock_t start = clock();
    int h;
    h = hash("i");
    DictSet(token_table, 1, h, -2);
    h = hash("read");
    DictSet(token_table, 4, h, -2);
    h = hash("the");
    DictSet(token_table, 3, h, -2);
    h = hash("paragraph");
    DictSet(token_table, 9, h, -2);
    h = hash("on");
    DictSet(token_table, 2, h, -2);
    h = hash("http");
    DictSet(token_table, 4, h, -2);
    h = hash("wikipedia");
    DictSet(token_table, 9, h, -2);
    h = hash("org");
    DictSet(token_table, 3, h, -2);
    // printf("%ld\n", clock() - start);
    int i, id;
    for (i = 0; i < n_mails; ++i) {
        parse_and_add_to_token_set(mails[i].subject, tokensets + mails[i].id,
                                   mails[i].id);
        parse_and_add_to_token_set(mails[i].content, tokensets + mails[i].id,
                                   mails[i].id);
    }
    int cnt = 0;
    int top;
    for (i = 0; i < n_mails; ++i) {
        // best with no reward bound 24
        top = (tokensets + i)->n_top;
        if ((top < 25)) {
            cnt++;
            similarity_available[i] = true;
            build_similarity_table(i);
            if (cnt >= 2000) {
                break;
            }
        }
        // printf("%d\n", (tokensets + i)->n_top);
    }
    // username preprocessing

    // current_id = 0;
    // for (i = 0; i < n_mails; ++i) {
    //     // h = hash(mails[i].from);
    //     // id = DictGet(username_table, strlen(mails[i].from), h);
    //     // if (id == -1) {
    //     //     id = current_id;
    //     //     DictSet(username_table, strlen(mails[i].from), h,
    //     //     current_id++);
    //     // }
    //     id = UserSetAdd(username_table, mails[i].from);
    //     edges[mails[i].id][0] = id;
    //     // h = hash(mails[i].to);
    //     // id = DictGet(username_table, strlen(mails[i].to), h);
    //     // if (id == -1) {
    //     //     id = current_id;
    //     //     DictSet(username_table, strlen(mails[i].to), h,
    //     //     current_id++);
    //     // }
    //     id = UserSetAdd(username_table, mails[i].to);
    //     edges[mails[i].id][1] = id;
    // }

    // double score = 0;
    for (i = 0; i < n_queries; ++i) {
        /*if (queries[i].type == expression_match) {*/
        /*    expression_match_query(*/
        /*        queries[i].id,*/
        /*        queries[i].data.expression_match_data.expression);*/
        /*    score += queries[i].reward;*/
        /*    fprintf(stderr, "%f\n", score);*/
        /*}*/
        // if (queries[i].type == group_analyse &&
        //     queries[i].reward * 125 >=
        //     queries[i].data.group_analyse_data.len) {
        //     group_analyse_query(queries[i].id,
        //                         queries[i].data.group_analyse_data.mids,
        //                         queries[i].data.group_analyse_data.len);
        // score += queries[i].reward;
        // fprintf(stderr, "%f\n", score);
        // }
        if (queries[i].type == find_similar &&
            similarity_available[queries[i].data.find_similar_data.mid]) {
            find_similar_query(queries[i].id,
                               queries[i].data.find_similar_data.mid,
                               queries[i].data.find_similar_data.threshold);
            // score += queries[i].reward;
            // fprintf(stderr, "%f\n", score);
        }
    }
    // for (i = 0; i < n_queries; ++i) {
    //     if (queries[i].type == group_analyse && queries[i].reward > 0.125) {
    //         group_analyse_query(queries[i].id,
    //                             queries[i].data.group_analyse_data.mids,
    //                             queries[i].data.group_analyse_data.len);
    //         // score += queries[i].reward;
    //         // fprintf(stderr, "%f\n", score);
    //     }
    // }

    return 0;
}
