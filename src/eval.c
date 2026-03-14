#include "lexer.h"
#include "parser.h"
#include "utils.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum { BUFFER_SIZE = 100 };

int evaluate (struct expr *, int *);

int main (void) {
    int value;
    char buffer[BUFFER_SIZE];
    struct expr *expression;
    struct token *tokens = calloc (MAX_TOKEN_NUM, sizeof (struct token));

    assert_nonnull (tokens, "ERROR: out of memory\n");

    printf (">> ");
    while (fgets (buffer, BUFFER_SIZE, stdin) != NULL) {
        if (lexer (tokens, buffer, strlen (buffer)) < 0) {
            fprintf (stderr, "ERROR: input string is too long\n");
            goto prompt;
        }

        if (parser (tokens, &expression) < 0) {
            fprintf (stderr, "ERROR: invalid syntax\n");
            goto prompt;
        }

        if (evaluate (expression, &value) < 0) {
            fprintf (stderr, "ERROR: division by zero\n");
            goto cleanup;
        }

        printf ("   = %d\n", value);

    cleanup:
        destroy_expr (expression);

    prompt:
        printf (">> ");
    }

    free (tokens);
    return 0;
}

/* Evaluates an expression that is represented by an abstract syntax tree named
 * `e`, writes the evaluated integer at `r`, and returns zero if successful. On
 * division-by-zero error, returns -1.*/
int evaluate (struct expr *e, int *r) {
    int left, right;

    if (e->type == EXPR_PAREN)
        return -1;

    if (e->type == EXPR_NUMBER) {
        *r = e->number;
        return 0;
    }

    assert (e->left != NULL && e->right != NULL);

    if (evaluate (e->left, &left) < 0 || evaluate (e->right, &right) < 0)
        return -1;

    switch (e->type) {
    case EXPR_PLUS:
        *r = left + right;
        break;

    case EXPR_MINUS:
        *r = left - right;
        break;

    case EXPR_MULTIPLY:
        *r = left * right;
        break;

    case EXPR_DIVIDE:
        if (right == 0)
            return -1;

        *r = left / right;
        break;

    default:
        /* This branch should never be reached. */
        assert (false);
    }

    return 0;
}
