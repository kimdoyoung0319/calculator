#include "lexer.h"
#include "parser.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum { BUFFER_SIZE = 100 };

int evaluate (struct expr *);

int main (void) {
    int result;
    char buffer[BUFFER_SIZE];
    struct expr *expression;
    struct token *tokens = calloc (MAX_TOKEN_NUM, sizeof (struct token));

    printf (">> ");
    while (fgets (buffer, BUFFER_SIZE, stdin) != NULL) {

        if (lexer (tokens, buffer, strlen (buffer)) == -1) {
            printf ("ERROR: input string is too long\n");
            continue;
        }

        expression = parser (tokens);
        result = evaluate (expression);
        destroy_expr (expression);
        printf ("   = %d\n>> ", result);
    }

    free (tokens);

    return 0;
}

/* Evaluates an expression that is represented by an abstract syntax tree named
 * 'e', returns the evaluated integer. */
int evaluate (struct expr *e) {
    int r, left, right;

    assert (e->type != EXPR_PAREN);

    if (e->type == EXPR_NUMBER)
        return e->number;

    assert (e->left != NULL && e->right != NULL);

    left = evaluate (e->left);
    right = evaluate (e->right);

    switch (e->type) {
    case EXPR_PLUS:
        r = left + right;
        break;

    case EXPR_MINUS:
        r = left - right;
        break;

    case EXPR_MULTIPLY:
        r = left * right;
        break;

    case EXPR_DIVIDE:
        r = left / right;
        break;

    default:
        /* This branch should never be reached. */
        assert (false);
    }

    return r;
}
