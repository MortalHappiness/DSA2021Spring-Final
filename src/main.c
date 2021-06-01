#include "api.h"

#define HASHSIZE 19697
#define INDEXSIZE 1000
// The testdata only contains the first 100 mails (mail1 ~ mail100)
// and 2000 queries for you to debug.

int n_mails, n_queries;
mail *mails;
query *queries;
float similarity[10000][10000] = {-1};

typedef struct hash_node {
    char *token;
    int len;
    struct hash_node *next;
} hash_node;

typedef struct mail_represent {
    int word;
    int index[INDEXSIZE];
    hash_node *hash_table[HASHSIZE];
} mail_represent;

mail_represent mails_list[10000];

void find_similarity(int id1, int id2) {
    if (similarity[id1][id2] != -1) {
        return similarity[id1][id2];
    }
    hash_node *tmp;
    int hit_count = 0;
    int total = 0;
    mail_represent mail = mails_list[id1];

    for (int i = 0; i < HASHSIZE; i++) {
        tmp = mail.hash_table[i];
        while (tmp != NULL) {
            // if (in()) }
        }
    }
}

int main(void) {
    api.init(&n_mails, &n_queries, &mails, &queries);

    char token_buffer[52];
    for (int i = 0; i < n_mails; i++) {
        // need to add hash function
        int idx = 0;
        int token_buffer_len = 0;
        int hash = 0;
        // ------------parse content [1024] ------------------
        while (mails[i].content[idx] != '\0') {
            if (mails[i].content[idx] >= 'a' && mails[i].content[idx] <= 'z') {
                token_buffer[token_buffer_len++] = mails[i].content[idx];
            } else if (mails[i].content[idx] >= 'A' &&
                       mails[i].content[idx] <= 'Z') {
                token_buffer[token_buffer_len++] = mails[i].content[idx];
            } else if (mails[i].content[idx] >= '0' &&
                       mails[i].content[idx] <= '9') {
                token_buffer[token_buffer_len++] = mails[i].content[idx];
            } else {
                if (token_buffer_len > 0) {
                    hash_node *new_node = malloc(sizeof(hash_node));
                    new_node->len = token_buffer_len;
                    new_node->token =
                        malloc((token_buffer_len + 1) * sizeof(char));
                    for (int j = 0; j < token_buffer_len; j++) {
                        new_node->token[j] = token_buffer[j];
                    }
                    new_node->token[token_buffer_len] = '\0';
                    if (mails_list[i].hash_table[hash] == NULL) {
                        new_node->next = NULL;
                        mails_list[i].hash_table[hash] = new_node;
                    } else {
                        new_node->next = mails_list[i].hash_table[hash];
                        mails_list[i].hash_table[hash] = new_node;
                    }
                    token_buffer_len = 0;
                    hash = 0;
                }
            }
            idx++;
        }
        if (token_buffer_len > 0) {
            hash_node *new_node = malloc(sizeof(hash_node));
            new_node->len = token_buffer_len;
            new_node->token = malloc((token_buffer_len + 1) * sizeof(char));
            for (int j = 0; j < token_buffer_len; j++) {
                new_node->token[j] = token_buffer[j];
            }
            new_node->token[token_buffer_len] = '\0';
            if (mails_list[i].hash_table[hash] == NULL) {
                new_node->next = NULL;
                mails_list[i].hash_table[hash] = new_node;
            } else {
                new_node->next = mails_list[i].hash_table[hash];
                mails_list[i].hash_table[hash] = new_node;
            }
            token_buffer_len = 0;
        }
        // ----------- parse subject [256] --------------
        idx = 0;
        token_buffer_len = 0;
        hash = 0;
        while (mails[i].subject[idx] != '\0') {
            if (mails[i].subject[idx] >= 'a' && mails[i].subject[idx] <= 'z') {
                token_buffer[token_buffer_len++] = mails[i].subject[idx];
            } else if (mails[i].subject[idx] >= 'A' &&
                       mails[i].subject[idx] <= 'Z') {
                token_buffer[token_buffer_len++] = mails[i].subject[idx];
            } else if (mails[i].subject[idx] >= '0' &&
                       mails[i].subject[idx] <= '9') {
                token_buffer[token_buffer_len++] = mails[i].subject[idx];
            } else {
                if (token_buffer_len > 0) {
                    hash_node *new_node = malloc(sizeof(hash_node));
                    new_node->len = token_buffer_len;
                    new_node->token =
                        malloc((token_buffer_len + 1) * sizeof(char));
                    for (int j = 0; j < token_buffer_len; j++) {
                        new_node->token[j] = token_buffer[j];
                    }
                    new_node->token[token_buffer_len] = '\0';
                    if (mails_list[i].hash_table[hash] == NULL) {
                        new_node->next = NULL;
                        mails_list[i].hash_table[hash] = new_node;
                    } else {
                        new_node->next = mails_list[i].hash_table[hash];
                        mails_list[i].hash_table[hash] = new_node;
                    }
                    token_buffer_len = 0;
                    hash = 0;
                }
            }
            idx++;
        }
        if (token_buffer_len > 0) {
            hash_node *new_node = malloc(sizeof(hash_node));
            new_node->len = token_buffer_len;
            new_node->token = malloc((token_buffer_len + 1) * sizeof(char));
            for (int j = 0; j < token_buffer_len; j++) {
                new_node->token[j] = token_buffer[j];
            }
            new_node->token[token_buffer_len] = '\0';
            if (mails_list[i].hash_table[hash] == NULL) {
                new_node->next = NULL;
                mails_list[i].hash_table[hash] = new_node;
            } else {
                new_node->next = mails_list[i].hash_table[hash];
                mails_list[i].hash_table[hash] = new_node;
            }
            token_buffer_len = 0;
        }
    }

    // for(int i = 0; i < n_queries; i++)
    // 	if(queries[i].type == expression_match)
    // 	  api.answer(queries[i].id, NULL, 0);

    return 0;
}
