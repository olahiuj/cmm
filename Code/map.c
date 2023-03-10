#include "map.h"
#include "common.h"

static mapent_t entries1[65536];
static mapent_t entries2[65536];
static mapent_t entries3[65536];

static node_t *node_alloc(const void *key, void *val) {
    node_t *node = zalloc(sizeof(node_t));
    node->key    = key;
    node->val    = val;
    node->h      = 1;
    return node;
}

static node_t *node_cpy(node_t *from) {
    node_t *node = zalloc(sizeof(node_t));
    *node        = *from;
    return node;
}

static i32 get_height(node_t *node) {
    return node ? node->h : 0;
}

static i32 get_balance(node_t *node) {
    ASSERT(node, "get_balance NULL");
    return get_height(node->lhs) - get_height(node->rhs);
}

static void calc_height(node_t *node) {
    node->h = max(get_height(node->lhs), get_height(node->rhs)) + 1;
}

static void map_to_array_helper(node_t *node, mapent_t *entries, u32 *index) {
    if (!node) return;
    map_to_array_helper(node->lhs, entries, index);
    entries[*index] = (mapent_t){
        .key = node->key,
        .val = node->val};
    *index = *index + 1;
    map_to_array_helper(node->rhs, entries, index);
}

u32 map_to_array(const map_t *map, mapent_t *entries) {
    u32 len = 0;
    map_to_array_helper(map->root, entries, &len);
    return len;
}

static node_t *map_from_array_helper(u32 len, mapent_t *entries) {
    if (len == 0) return NULL;
    if (len == 1) return node_alloc(entries->key, entries->val);
    node_t *node = node_alloc(entries[len / 2].key, entries[len / 2].val);

    node->lhs = map_from_array_helper(len / 2, entries);
    node->rhs = map_from_array_helper(len - len / 2 - 1, &entries[len / 2 + 1]);
    calc_height(node);
    return node;
}

void map_from_array(map_t *map, u32 len, mapent_t *entries) {
    *map = (map_t){
        .root = map_from_array_helper(len, entries),
        .size = len};
}

void map_init(map_t *map) {
    ASSERT(map != NULL, "map NULL");
    map->root = NULL;
    map->size = 0;
}

static void map_fini_helper(map_t *map, map_iter_t iter) {
    if (iter.done) return;
    node_t *node = iter.node;
    map_iter_next(&iter);
    map_fini_helper(map, iter);
    zfree(node);
}

void map_fini(map_t *map) {
    map_fini_helper(map, map_iter_init(map));
}

static i32 cmp(const void *lhs, const void *rhs) {
    if ((uptr) lhs < (uptr) rhs) {
        return -1;
    } else if ((uptr) lhs > (uptr) rhs) {
        return 1;
    }
    return 0;
}

static node_t *map_find_helper(const map_t *map, node_t *node, const void *key) {
    if (!node) {
        return NULL;
    }
    i32 cmp_val = cmp(key, node->key);
    if (!cmp_val) {
        return node;
    } else if (cmp_val > 0) {
        return map_find_helper(map, node->rhs, key);
    } else {
        return map_find_helper(map, node->lhs, key);
    }
}

void *map_find(const map_t *map, const void *key) {
    node_t *node = map_find_helper(map, map->root, key);
    if (!node) {
        return NULL;
    }
    return node->val;
}

static void node_validate(node_t *node) {
#ifndef NDEBUG
    if (!node) return;
    node_validate(node->lhs);
    node_validate(node->rhs);
    u32 h = node->h;
    calc_height(node);
    ASSERT(h == node->h, "height");
#endif
}

static node_t *rotate_R(node_t *const node) {
    node_t *lhs = node->lhs;
    node->lhs   = lhs->rhs;
    lhs->rhs    = node;
    calc_height(node);
    calc_height(lhs);
    return lhs;
}

static node_t *rotate_L(node_t *const node) {
    node_t *rhs = node->rhs;
    node->rhs   = rhs->lhs;
    rhs->lhs    = node;
    calc_height(node);
    calc_height(rhs);
    return rhs;
}

static node_t *rebalance(node_t *node) {
    if (!node) return NULL;
    calc_height(node);
    i32 b = get_balance(node);
    if (b == 2) {
        if (get_balance(node->lhs) < 0) {
            node->lhs = rotate_L(node->lhs);
        }
        return rotate_R(node);
    } else if (b == -2) {
        if (get_balance(node->rhs) == 1) {
            node->rhs = rotate_R(node->rhs);
        }
        return rotate_L(node);
    }
    return node;
}

static node_t *map_insert_helper(map_t *map, node_t **pnode, const void *key) {
    node_t *node    = *pnode;
    i32     cmp_val = cmp(key, node->key);
    if (!cmp_val) {
        return node;
    }
    node_t *new = NULL;
    if (cmp_val > 0) {
        if (!node->rhs) {
            new = node_alloc(key, NULL);
            map->size++;
            node->rhs = new;
        } else {
            new = map_insert_helper(map, &node->rhs, key);
        }
    } else {
        if (!node->lhs) {
            new = node_alloc(key, NULL);
            map->size++;
            node->lhs = new;
        } else {
            new = map_insert_helper(map, &node->lhs, key);
        }
    }
    *pnode = rebalance(node);
    calc_height(node);
    return new;
}

void map_insert(map_t *map, const void *key, void *val) {
    ASSERT(key != NULL, "insert key NULL");
    if (val == NULL) {
        map_remove(map, key);
        return;
    }
    if (!map->root) {
        map->root = node_alloc(key, val);
        map->size++;
    } else {
        node_t *node = map_insert_helper(map, &map->root, key);
        ASSERT(node != NULL, "insert NULL");
        node->val = val;
    }
    map->root = rebalance(map->root);
    node_validate(map->root);
}

static node_t *get_succ(node_t *node) {
    ASSERT(node->rhs, "node->rhs");
    node = node->rhs;
    while (node->lhs) {
        node = node->lhs;
    }
    return node;
}

static void map_remove_helper(map_t *map, node_t **pnode, const void *key) {
    node_t *node = *pnode;
    if (!node) return; // remove failed
    i32 cmp_val = cmp(key, node->key);
    if (!cmp_val) {
        if (node->h == 1) { // leaf
            zfree(node);
            *pnode = NULL;
            map->size--;
            return;
        } else if (node->lhs == NULL) {
            swap(node->key, node->rhs->key);
            swap(node->val, node->rhs->val);
            map_remove_helper(map, &node->rhs, key);
        } else if (node->rhs == NULL) {
            swap(node->key, node->lhs->key);
            swap(node->val, node->lhs->val);
            map_remove_helper(map, &node->lhs, key);
        } else {
            node_t *succ = get_succ(node);
            swap(node->key, succ->key);
            swap(node->val, succ->val);
            map_remove_helper(map, &node->rhs, key);
        }
    } else if (cmp_val > 0) {
        map_remove_helper(map, &node->rhs, key);
    } else {
        map_remove_helper(map, &node->lhs, key);
    }
    if (node->lhs) node->lhs = rebalance(node->lhs);
    if (node->rhs) node->rhs = rebalance(node->rhs);
    calc_height(node);
}

void map_remove(map_t *map, const void *key) {
    map_remove_helper(map, &map->root, key);
    map->root = rebalance(map->root);
}

void set_init(set_t *set) {
    map_init(set);
}

void set_fini(set_t *set) {
    map_fini(set);
}

void set_insert(set_t *set, void *elem) {
    map_insert(set, elem, elem);
}

bool set_contains(const set_t *set, const void *elem) {
    return map_find(set, elem) != NULL;
}

bool set_eq(set_t *lhs, set_t *rhs) {
    if (lhs == rhs) return true;
    if (lhs->size != rhs->size) return false;
    set_iter(lhs, it) {
        if (it.val && !set_contains(rhs, it.val)) {
            return false;
        }
    }
    return true;
}

bool map_eq(map_t *lhs, map_t *rhs) {
    if (lhs == rhs) return true;
    if (lhs->size != rhs->size) return false;
    map_iter(lhs, it) {
        if (it.val && it.val != map_find(rhs, it.key)) {
            return false;
        }
    }
    return true;
}

void set_cpy(set_t *dst, set_t *src) {
    map_cpy(dst, src);
}

static node_t *map_cpy_helper(node_t *node) {
    if (!node) return NULL;
    node_t *new_node = node_cpy(node);
    new_node->lhs    = map_cpy_helper(node->lhs);
    new_node->rhs    = map_cpy_helper(node->rhs);
    return new_node;
}

void map_cpy(map_t *dst, map_t *src) {
    map_init(dst);
    dst->root = map_cpy_helper(src->root);
    dst->size = src->size;
}

void set_remove(set_t *set, const void *elem) {
    map_remove(set, elem);
}

bool set_merge(set_t *into, const set_t *rhs) {
    u32  len_into = map_to_array(into, entries1);
    u32  len_rhs  = map_to_array(rhs, entries2);
    bool changed  = false;

    u32 i = 0, j = 0, len = 0;
    while (i < len_into && j < len_rhs) {
        mapent_t ent_into = entries1[i];
        mapent_t ent_rhs  = entries2[j];
        if ((uptr) ent_into.key < (uptr) ent_rhs.key) {
            entries3[len++] = ent_into;
            i++;
        } else if ((uptr) ent_into.key > (uptr) ent_rhs.key) {
            entries3[len++] = ent_rhs;
            j++;
            changed = true;
        } else {
            entries3[len++] = (mapent_t){
                .key = ent_into.key,
                .val = ent_into.val};
            i++;
            j++;
        }
    }
    while (i < len_into) {
        entries3[len++] = entries1[i++];
    }
    while (j < len_rhs) {
        entries3[len++] = entries2[j++];
        changed         = true;
    }
    set_fini(into);
    map_from_array(into, len, entries3);
    ASSERT(i <= sizeof(entries1), "entries_into overflow");
    ASSERT(j <= sizeof(entries2), "entries_rhs overflow");
    ASSERT(len <= sizeof(entries3), "entries overflow");
    return changed;
}

bool set_intersect(set_t *into, const set_t *rhs) {
    u32  len_into = map_to_array(into, entries1);
    u32  len_rhs  = map_to_array(rhs, entries2);
    bool changed  = false;

    u32 i = 0, j = 0, len = 0;
    while (i < len_into && j < len_rhs) {
        mapent_t ent_into = entries1[i];
        mapent_t ent_rhs  = entries2[j];
        if ((uptr) ent_into.key < (uptr) ent_rhs.key) {
            i++;
            changed = true;
        } else if ((uptr) ent_into.key > (uptr) ent_rhs.key) {
            j++;
        } else {
            entries3[len++] = (mapent_t){
                .key = ent_into.key,
                .val = ent_into.val};
            i++;
            j++;
        }
    }
    changed |= (i < len_into);
    set_fini(into);
    map_from_array(into, len, entries3);
    ASSERT(i <= sizeof(entries1), "entries_into overflow");
    ASSERT(j <= sizeof(entries2), "entries_rhs overflow");
    ASSERT(len <= sizeof(entries3), "entries overflow");
    return changed;
}

void map_merge(map_t *into, const map_t *rhs) {
    map_iter(rhs, it) {
        map_insert(into, it.key, it.val);
    }
}

bool map_intersect(map_t *into, const map_t *rhs) {
    map_t result;
    map_init(&result);
    u32 size = into->size;
    map_iter(into, it) {
        void *val = map_find(rhs, it.key);
        if (val == it.val) {
            map_insert(&result, it.key, it.val);
        }
    }
    map_fini(into);
    map_cpy(into, &result);
    return into->size != size;
}

map_iter_t map_iter_init(const map_t *map) {
    map_iter_t iter = (map_iter_t){
        .node = map->root,
        .done = false};
    if (!iter.node) { // empty map
        iter.done = true;
    } else {
        map->root->next = NULL;
        iter.key        = iter.node->key;
        iter.val        = iter.node->val;
    }
    return iter;
}

void map_iter_next(map_iter_t *iter) {
    node_t *front = iter->node;
    iter->node    = front->next;
    if (front->lhs) {
        front->lhs->next = iter->node;
        iter->node       = front->lhs;
    }
    if (front->rhs) {
        front->rhs->next = iter->node;
        iter->node       = front->rhs;
    }
    if (!iter->node) {
        iter->key  = NULL;
        iter->val  = NULL;
        iter->done = true;
        return;
    }
    iter->key = iter->node->key;
    iter->val = iter->node->val;
}

set_iter_t set_iter_init(const set_t *set) {
    return map_iter_init(set);
}

void set_iter_next(set_iter_t *iter) {
    map_iter_next(iter);
}