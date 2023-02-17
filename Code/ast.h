#pragma once

#include "common.h"
#include "symtab.h"
#include "cst.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#define VISITOR_METHOD_ASSIGN(NAME) \
    .visit_##NAME = (void *) visit_##NAME,
#define VISITOR_METHOD_DECLARE(NAME, ARG_TYPE) \
    static void visit_##NAME(ast_t *node, ARG_TYPE);
#define VISITOR_DEF(NAME, ARG_TYPE)                                  \
    AST_NODES_WITH_ARG(VISITOR_METHOD_DECLARE, ARG_TYPE)             \
    const struct ast_visitor visitor_##NAME = (struct ast_visitor) { \
        .name = STRINGIFY(NAME),                                     \
        AST_NODES(VISITOR_METHOD_ASSIGN)                             \
    }

#define ast_foreach(NODE, IT) \
    for (ast_t * (IT) = (NODE); (NODE) != NULL && (IT) != NULL; (IT) = (IT)->next)

#define sym_foreach(SYM, IT) \
    for (syment_t * (IT) = (void *) (SYM); (SYM) != NULL && (IT) != NULL; (IT) = (IT)->next)

typedef struct ast_t ast_t;

typedef enum {
    AST_NODES(LIST)
} ast_kind_t;

typedef enum {
    OPS(LIST)
} op_kind_t;

extern const char *AST_NODE_NAMES[];

#define SYMS_EXTEND(SYM) \
    typedef struct SYM##_entry SYM##_entry;

SYMS(SYMS_EXTEND)

struct ast_t {
    ast_t     *next;
    u32        fst_l;
    ast_kind_t ast_kind;
};

#define AST_NODE_EXTEND(NODE) \
    typedef struct NODE##_node_t NODE##_node_t;

AST_NODES(AST_NODE_EXTEND);

struct CONS_PROG_node_t {
    EXTENDS(ast_t);
    ast_t *decls;
};

/* Types */
typedef struct type_t {
    enum kind_t {
        TYPE_PRIM_INT,
        TYPE_PRIM_FLT,
        TYPE_PRIM_BOT,
        TYPE_STRUCT
    } spec_typ;
    char   str[MAX_SYM_LEN];
    ast_t *decls;
    u32    dim;
} type_t;

#define IS_SCALAR(TYPE)                       \
    ((TYPE).dim == 0)                         \
        && (((TYPE).spec_typ == TYPE_PRIM_INT \
             || (TYPE).spec_typ == TYPE_PRIM_FLT))

#define IS_LOGIC(TYPE)                  \
    (((TYPE).spec_typ == TYPE_PRIM_INT) \
     && IS_SCALAR(TYPE))

/* Stmts */
struct STMT_SCOP_node_t {
    EXTENDS(ast_t);
    ast_t *decls, *stmts;
};

struct STMT_IFTE_node_t {
    EXTENDS(ast_t);
    ast_t *cond, *tru_stmt, *fls_stmt;
};

struct STMT_WHLE_node_t {
    EXTENDS(ast_t);
    ast_t *cond, *body;
};

struct STMT_EXPR_node_t {
    EXTENDS(ast_t);
    ast_t *expr;
};

struct STMT_RET_node_t {
    EXTENDS(ast_t);
    ast_t *expr;
};

/* Exprs */
struct EXPR_IDEN_node_t {
    EXTENDS(ast_t);
    SYM_VAR_entry *sym;
    char           str[MAX_SYM_LEN];
};

struct EXPR_CALL_node_t {
    EXTENDS(ast_t);
    SYM_FUN_entry *fun;
    ast_t         *expr; // nullable
    char           str[MAX_SYM_LEN];
};

struct EXPR_ASS_node_t {
    EXTENDS(ast_t);
    ast_t *lhs, *rhs;
};

struct EXPR_ARR_node_t {
    EXTENDS(ast_t);
    ast_t *arr, *ind;
};

struct EXPR_DOT_node_t {
    EXTENDS(ast_t);
    char   str[MAX_SYM_LEN];
    ast_t *base;
};

struct EXPR_INT_node_t {
    EXTENDS(ast_t);
    i32 value;
};

struct EXPR_FLT_node_t {
    EXTENDS(ast_t);
    f32 value;
};

struct EXPR_BIN_node_t {
    EXTENDS(ast_t);
    ast_t    *lhs;
    ast_t    *rhs;
    op_kind_t op;
};

struct EXPR_UNR_node_t {
    EXTENDS(ast_t);
    ast_t    *sub;
    op_kind_t op;
};

struct DECL_VAR_node_t {
    EXTENDS(ast_t);
    SYM_VAR_entry *sym;
    type_t         typ;
    ast_t         *expr; // init val
    char           str[MAX_SYM_LEN];
    u32            dim;
};

struct DECL_FUN_node_t {
    EXTENDS(ast_t);
    char   str[MAX_SYM_LEN];
    ast_t *params;
    type_t typ;
    ast_t *body;
};

struct DECL_TYP_node_t {
    EXTENDS(ast_t);
    type_t typ;
};

struct SYM_TYP_entry {
    EXTENDS(syment_t);
    type_t typ;
};

struct SYM_VAR_entry {
    EXTENDS(syment_t);
    type_t typ;
};

struct SYM_FUN_entry {
    EXTENDS(syment_t);
    type_t         typ;
    SYM_VAR_entry *params;
};

typedef void (*ast_visitor_fun_t)(ast_t *, void *);

#define AST_NODE_VISIT(NODE) \
    ast_visitor_fun_t visit_##NODE;

struct ast_visitor {
    char name[MAX_SYM_LEN];
    AST_NODES(AST_NODE_VISIT)
};

#define INSTANCE_OF(NODE, KIND)                    \
    ASSERT(NODE->ast_kind == KIND,                 \
           "instanceof %s", AST_NODE_NAMES[KIND]); \
    KIND##_node_t *cnode = (KIND##_node_t *) NODE;

#define INSTANCE_OF_VAR(NODE, KIND, CNODE)         \
    ASSERT(NODE->ast_kind == KIND,                 \
           "instanceof %s", AST_NODE_NAMES[KIND]); \
    KIND##_node_t *CNODE = (KIND##_node_t *) NODE;

ast_t *ast_alloc(ast_kind_t kind, u32 fst_l, ...);

void ast_free(ast_t *node);

void ast_check(ast_t *node, type_t *typ);

void visitor_dispatch(const struct ast_visitor visitor, ast_t *node, void *p);