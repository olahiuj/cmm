#pragma once

#include "ast.h"
#include "common.h"

#define IR(F)    \
    F(IR_LABEL)  \
    F(IR_ASSIGN) \
    F(IR_BINARY) \
    F(IR_DREF)   \
    F(IR_LOAD)   \
    F(IR_STORE)  \
    F(IR_GOTO)   \
    F(IR_BRANCH) \
    F(IR_RETURN) \
    F(IR_DEC)    \
    F(IR_ARG)    \
    F(IR_CALL)   \
    F(IR_PARAM)  \
    F(IR_READ)   \
    F(IR_WRITE)

#define IR_PURE(F) \
    F(IR_ASSIGN)   \
    F(IR_BINARY)   \
    F(IR_DREF)     \
    F(IR_LOAD)

#define IR_WITH_ARG(F, ARG) \
    F(IR_LABEL, ARG)        \
    F(IR_ASSIGN, ARG)       \
    F(IR_BINARY, ARG)       \
    F(IR_DREF, ARG)         \
    F(IR_LOAD, ARG)         \
    F(IR_STORE, ARG)        \
    F(IR_GOTO, ARG)         \
    F(IR_BRANCH, ARG)       \
    F(IR_RETURN, ARG)       \
    F(IR_DEC, ARG)          \
    F(IR_ARG, ARG)          \
    F(IR_CALL, ARG)         \
    F(IR_PARAM, ARG)        \
    F(IR_READ, ARG)         \
    F(IR_WRITE, ARG)

typedef enum {
    IR_NULL,
    IR(LIST)
} ir_kind_t;

typedef struct IR_list  ir_list;
typedef struct IR_fun_t ir_fun_t;
typedef struct IR_t     IR_t;
typedef struct oprd_t   oprd_t;
typedef struct chain_t  chain_t;

typedef enum {
    OPRD_LIT,
    OPRD_VAR,
} oprd_kind_t;

struct oprd_t {
    const char *name;
    oprd_kind_t kind;
    union {
        i64  val;
        uptr id;
    };
    u32  lineno;
    uptr offset;
};

struct IR_t {
    EXTENDS(shared);
    char      str[MAX_SYM_LEN];
    ir_kind_t kind;
    oprd_t    tar, lhs, rhs;
    op_kind_t op;
    IR_t     *prev, *next, *jmpto;
    u32       id;
    bool      mark;

    struct block_t *parent;
};

struct chain_t {
    IR_t    *ir;
    chain_t *next;
};

struct IR_list {
    IR_t    *head, *tail;
    chain_t *fls, *tru;
    u32      size;
};

struct IR_fun_t {
    char      str[MAX_SYM_LEN];
    ir_list   instrs;
    ir_fun_t *next;
    u32       sf_size;
};

void ir_append(ir_list *list, IR_t *ir);

void ir_prepend(ir_list *list, IR_t *ir);

void ir_concat(ir_list *front, const ir_list back);

void ir_validate(const ir_list *list);

void ir_remove_mark(ir_list *list);

ir_list ir_split(ir_list *list, IR_t *it);

typedef void (*ir_visitor_fun_t)(IR_t *, void *);

#define IR_VISIT(NAME) ir_visitor_fun_t visit_##NAME;

struct IR_visitor {
    char name[MAX_SYM_LEN];
    IR(IR_VISIT)
};

#define IR_EXTEND(NAME) typedef struct IR_t NAME##_t;

IR(IR_EXTEND);

void ir_fun_print(FILE *file, ir_fun_t *fun);

void ir_list_free(ir_list *list);

void ir_print(FILE *file, IR_t *ir);

IR_t *ir_alloc(ir_kind_t kind, ...);

IR_t *ir_dup(IR_t *ir);

void ir_check(ir_list *list);

i32 oprd_cmp(const void *lhs, const void *rhs);

oprd_t var_alloc(const char *name, u32 lineno);

oprd_t lit_alloc(i64 value);

void ir_fun_free(ir_fun_t *fun);

char *oprd_to_str(oprd_t oprd);

ir_list ast_gen(AST_t *node, oprd_t tar);

void chain_insert(chain_t **chain, IR_t *ir);

void chain_resolve(chain_t **chain, IR_t *ir);

void chain_merge(chain_t **into, chain_t *rhs);