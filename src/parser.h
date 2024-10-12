#ifndef __PARSER_H__
#define __PARSER_H__

#include <assert.h>
#include <stdbool.h>

enum token_type {
	TOKEN_NULL,
	TOKEN_NUMBER,
	TOKEN_PLUS,
	TOKEN_MINUS,
	TOKEN_MULTIPLY,
	TOKEN_DIVIDE,
	TOKEN_LEFT_PAREN,
	TOKEN_RIGHT_PAREN,
};

enum associativity {
	ASSOC_LEFT,
	ASSOC_RIGHT,
	ASSOC_NONE,
};

extern const int MAX_TOKEN_NUM;
extern const int TOKEN_PRECEDENCES[];
extern const enum associativity TOKEN_ASSOCIATIVITY[];

struct token {
	int number;
	enum token_type type;
};

struct token *parser (const char *, int);
bool is_token_null (struct token *);
bool is_operator_token (struct token *);
int token_type_to_precedence (enum token_type);

#endif