#include "symtab.h"
#include "common.h"
#include <string.h>

#define MAX_ENTRY 4096

static hashtab_t *top  = NULL;
static bool       init = false;
static syment_t   entries[MAX_ENTRY];

static u32 str_hash(const char *s) {
    u32 hash = 0;
    for (const char *p = s; *p; p++) {
        hash = (hash * 17 + *p) % MAX_SYM;
    }
    return hash;
}

static syment_t **lookup(hashtab_t *hashtab, const char *str) {
    syment_t **bucket = hashtab->bucket;
    u32        hash   = str_hash(str);
    while (bucket[hash] && strcmp(bucket[hash]->str, str)) {
        hash = (hash + 1) % MAX_SYM;
    }
    return &bucket[hash];
}

syment_t *sym_lookup(const char *str) {
    ASSERT(init, "symtab used before initialized");
    for (hashtab_t *cur = top; cur; cur = cur->prev) {
        syment_t *sym = *lookup(cur, str);
        if (sym && !strcmp(sym->str, str)) {
            return sym;
        }
    }
    return NULL;
}

void sym_scope_push() {
    ASSERT(init, "symtab used before initialized");
    hashtab_t *hashtab = zalloc(sizeof(hashtab_t));
    hashtab->prev      = top;
    top                = hashtab;
}

void sym_scope_pop() {
    ASSERT(init, "symtab used before initialized");
    hashtab_t *hashtab = top;
    top                = top->prev;
    zfree(hashtab);
}

syment_t *salloc() {
    static syment_t *ptr = entries;
    return ptr++;
}

void sym_insert(const char *str, void *data, u32 fst_l, u32 fst_c) {
    ASSERT(init, "symtab used before initialized");
    ASSERT(strlen(str) < MAX_SYM_LEN, "sym_insert exceeds MAX_SYM_LEN")
    syment_t **sym = lookup(top, str);
    ASSERT(*sym == NULL, "sym inserted twice");

    *sym  = salloc();
    **sym = (syment_t){
        .data  = data,
        .fst_c = fst_c,
        .fst_l = fst_l};
    memcpy((*sym)->str, str, strlen(str));
}

void symtab_init() {
    init = true;
    sym_scope_push();
}

void symtab_fini() {
    while (top) {
        sym_scope_pop();
    }
    init = false;
}

void symcpy(char *dst, const char *src) {
    strncpy(dst, src, MAX_SYM_LEN - 1);
}

void symmov(char *dst, char *src) {
    symcpy(dst, src);
    zfree(src);
}

int symcmp(const char *x, const char *y) {
    return strncmp(x, y, MAX_SYM_LEN);
}