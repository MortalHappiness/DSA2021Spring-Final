#include "api.h"

#define MAX_NMAILS 10000

// ========================================
// PRECOMPUTED VALUES

// const char INTERSECTION_TABLE_STR[];
// const int TOKENSET_SIZE[10000];

// ========================================
// Global variables

int n_mails, n_queries;
mail *mails;
query *queries;

int answer[MAX_NMAILS];

double SIMILARITY_TABLE[MAX_NMAILS][MAX_NMAILS] = {0};

// ========================================

void find_similar_query(int query_id, int mail_id, double threshold) {
    int i, answer_length;

    answer_length = 0;
    double *row = SIMILARITY_TABLE[mail_id];
    for (i = 0; i < mail_id; ++i) {
        if (row[i] > threshold)
            answer[answer_length++] = i;
    }
    for (i = i + 1; i < n_mails; ++i) {
        if (row[i] > threshold)
            answer[answer_length++] = i;
    }
    api.answer(query_id, answer, answer_length);
}

// ========================================

void parse_intersection_table() {
    // space is 32
    char lower = '#';             // 35
    char upper = '[';             // 91
    int base = upper - lower + 1; // 57

    int i, j, k, cur, n;
    char c;
    cur = 0;
    n = strlen(INTERSECTION_TABLE_STR);
    i = 0;
    j = 1;
    for (k = 0; k < n; ++k) {
        c = INTERSECTION_TABLE_STR[k];
        if (c == ' ') {
            SIMILARITY_TABLE[i][j] = SIMILARITY_TABLE[j][i] =
                (double)cur / (TOKENSET_SIZE[i] + TOKENSET_SIZE[j] - cur);
            ++j;
            if (j == MAX_NMAILS) {
                ++i;
                j = i + 1;
            }
            cur = 0;
        } else {
            cur = base * cur + (c - lower);
        }
    }
}

// ========================================

int main(void) {
    api.init(&n_mails, &n_queries, &mails, &queries);

    int i, id;

    parse_intersection_table();

    // double score = 0;
    for (i = 0; i < n_queries; ++i) {
        if (queries[i].type == find_similar && queries[i].reward < 70) {
            find_similar_query(queries[i].id,
                               queries[i].data.find_similar_data.mid,
                               queries[i].data.find_similar_data.threshold);
            // score += queries[i].reward;
            // fprintf(stderr, "%f\n", score);
        }
    }

    return 0;
}
