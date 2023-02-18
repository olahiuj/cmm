#pragma once

#include "common.h"
#include <stdarg.h>

typedef struct type_t type_t;

typedef struct field_t field_t;

struct type_t {
    enum type_kind {
        TYPE_ERR = 0,
        TYPE_PRIM_INT,
        TYPE_PRIM_FLT,
        TYPE_STRUCT,
        TYPE_ARRAY
    } kind;
    char     str[MAX_SYM_LEN];
    field_t *fields;
    u32      dim;
};

struct field_t {
    field_t *next;
    char     str[MAX_SYM_LEN];
    type_t   typ;
};

#define IS_SCALAR(TYPE)           \
    ((TYPE).kind == TYPE_PRIM_INT \
     || (TYPE).kind == TYPE_PRIM_FLT)

#define IS_LOGIC(TYPE)              \
    (((TYPE).kind == TYPE_PRIM_INT) \
     && IS_SCALAR(TYPE))

field_t *field_alloc(type_t typ, const char str[]);

void field_free(field_t *field);

type_t *typ_alloc(enum type_kind kind, ...);

void typ_free(type_t typ);
