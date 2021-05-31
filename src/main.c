#include "api.h"
#include <stdio.h>

// The testdata only contains the first 100 mails (mail1 ~ mail100)
// and 2000 queries for you to debug.

int n_mails, n_queries;
mail *mails;
query *queries;

typedef struct hash_node{
	char* token;
	int len;
	struct hash_node *next;
}hash_node;

int main(void) {
	api.init(&n_mails, &n_queries, &mails, &queries);
	char token_buffer[32];
	for (int i = 0; i < 1 ; i++){ //n_mails
		int idx = 0;
		int token_buffer_len = 0;
		while(mails[i].content[idx] != '\0'){

			if(mails[i].content[idx] >= 'a' && mails[i].content[idx] <= 'z'){
				token_buffer[token_buffer_len++] = mails[i].content[idx];
			}else if(mails[i].content[idx] >= 'A' && mails[i].content[idx] <= 'Z'){
				token_buffer[token_buffer_len++] = mails[i].content[idx];
			}else if(mails[i].content[idx] >= '0' && mails[i].content[idx] <= '9'){
				token_buffer[token_buffer_len++] = mails[i].content[idx];
			}else{
				if (token_buffer_len > 0){
					hash_node *new_node = malloc(sizeof(hash_node));
					new_node->len = token_buffer_len;
					new_node->token = malloc((token_buffer_len + 1) * sizeof(char));
					for (int j  =0; j < token_buffer_len;j++){
						new_node->token[j] = token_buffer[j];
					}
					new_node->next = NULL;
					new_node->token[token_buffer_len] = '\0';
					printf("%s\n", new_node->token);
					token_buffer_len = 0;
				}
				
			}
			idx++;
		}
	}
	
	// for(int i = 0; i < n_queries; i++)
	// 	if(queries[i].type == expression_match)
	// 	  api.answer(queries[i].id, NULL, 0);

  return 0;
}
