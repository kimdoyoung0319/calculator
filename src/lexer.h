#ifndef __LEXER_H__
#define __LEXER_H__

#include "parser.h"

enum expr_type {
	EXPR_PAREN,
	EXPR_NUMBER,
	EXPR_PLUS,
	EXPR_MINUS,
	EXPR_MULTIPLY,
	EXPR_DIVIDE,
};

struct expr {
	int number;
	enum expr_type type;
	struct expr *left, *right;
};

struct expr *lexer (struct token *);
struct expr *make_expr (enum expr_type, struct expr *, struct expr *);
void destroy_expr (struct expr *);

#endif