#include "visitor.h"
#include "common.h"
#include <stdbool.h>

#define RET_TYPE bool *
#define ARG res
VISITOR_DEF(AST, lval, bool *);

bool ast_lval(AST_t *node) {
    bool ARG;
    VISITOR_DISPATCH(AST, lval, node, &ARG);
    return ARG;
}

VISIT(EXPR_INT) {
    RETURN(false);
}

VISIT(EXPR_FLT) {
    RETURN(false);
}

VISIT(EXPR_BIN) {
    RETURN(false);
}

VISIT(EXPR_UNR) {
    RETURN(false);
}

VISIT(EXPR_IDEN) {
    RETURN(true);
}

VISIT(STMT_RET) {
    RETURN(false);
}

VISIT(STMT_WHLE) {
    RETURN(false);
}

VISIT(STMT_IFTE) {
    RETURN(false);
}

VISIT(STMT_SCOP) {
    RETURN(false);
}

VISIT(EXPR_DOT) {
    RETURN(true);
}

VISIT(EXPR_ASS) {
    RETURN(false);
}

VISIT(CONS_PROG) {
    RETURN(false);
}

VISIT(CONS_FUN) {
    RETURN(false);
}

VISIT(DECL_VAR) {
    RETURN(false);
}

VISIT(EXPR_ARR) {
    RETURN(true);
}

VISIT(STMT_EXPR) {
    RETURN(false);
}

VISIT(EXPR_CALL) {
    RETURN(false);
}

VISIT(DECL_TYP) {
    RETURN(false);
}

VISIT(CONS_SPEC) {
    RETURN(false);
}