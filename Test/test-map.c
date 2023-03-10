#include "common.h"
#include "map.h"
#include <string.h>

static void test_insert() {
    static char *str_arr[] = {
        "ABC", "123", "__009"};
    map_t map;
    map_init(&map, (void *) strcmp);

    for (i32 i = 0; i < ARR_LEN(str_arr); i++) {
        map_insert(&map, str_arr[i], str_arr[i]);
    }
    ASSERT(map_find(&map, "ABC") == str_arr[0], "ABC should be present");
    map_insert(&map, "ABC", test_insert);
    ASSERT(map_find(&map, "ABC") == test_insert, "ABC should be modified");
    map_remove(&map, "ABC");
    ASSERT(map_find(&map, "ABC") == NULL, "ABC should be deleted");
    ASSERT(map_find(&map, "114514") == NULL, "114514 should not be found");
    map_fini(&map);
}

static i32 cmp(const void *lhs, const void *rhs) {
    return ((uptr) lhs) - ((uptr) rhs);
}

static void test_as_array() {
    map_t map;
    map_init(&map, cmp);

    map_insert(&map, (void *) 1, (void *) 1);
    ASSERT(map_find(&map, (void *) 1) == (void *) 1, "insert failed");
    map_insert(&map, (void *) 1, (void *) 9);
    ASSERT(map_find(&map, (void *) 1) == (void *) 9, "insert failed");
    map_insert(&map, (void *) 4, (void *) 1);
    ASSERT(map_find(&map, (void *) 4) == (void *) 1, "insert failed");
    map_insert(&map, (void *) 5, (void *) 9);
    ASSERT(map_find(&map, (void *) 5) == (void *) 9, "insert failed");
    map_insert(&map, (void *) 1, (void *) 8);
    ASSERT(map_find(&map, (void *) 1) == (void *) 8, "insert failed");
    map_insert(&map, (void *) 4, (void *) 1);
    ASSERT(map_find(&map, (void *) 4) == (void *) 1, "insert failed");

    map_iter(&map, it) {
        printf("%lu", (uptr) it.val);
    }
    puts("");
    map_fini(&map);
}

static void test_set() {
    set_t set;
    set_init(&set, cmp);

    set_insert(&set, (void *) 1);
    set_iter(&set, it) printf("%lu ", (uptr) it.val);
    puts("");
    set_insert(&set, (void *) 4);
    set_iter(&set, it) printf("%lu ", (uptr) it.val);
    puts("");
    set_insert(&set, (void *) 2);

    ASSERT(set_contains(&set, (void *) 2), "missing 2");
    set_iter(&set, it) printf("%lu ", (uptr) it.val);
    puts("");
    set_fini(&set);
}

static void test_merge() {
    set_t lhs, rhs;
    set_init(&lhs, cmp);
    set_init(&rhs, cmp);

    set_insert(&lhs, (void *) 1);
    set_insert(&lhs, (void *) 2);
    set_insert(&lhs, (void *) 3);

    set_insert(&rhs, (void *) 2);
    set_insert(&rhs, (void *) 4);
    set_insert(&rhs, (void *) 3);

    set_merge(&lhs, &rhs);

    set_iter(&lhs, it) {
        printf("%lu\n", (uptr) it.val);
    }
    ASSERT(lhs.size == 4, "lhs size err");
}

static void print_seq(node_t *node) {
    if (!node) return;
    print_seq(node->lhs);
    printf("%lu ", (uptr) node->key);
    print_seq(node->rhs);
}

static void test_seq() {
    set_t set;
    set_init(&set, cmp);

    set_insert(&set, (void *) 5);
    print_seq(set.root);
    puts("");
    set_insert(&set, (void *) 4);
    print_seq(set.root);
    puts("");
    set_insert(&set, (void *) 2);
    print_seq(set.root);
    puts("");
    set_insert(&set, (void *) 3);
    print_seq(set.root);
    puts("");
    set_remove(&set, (void *) 5);
    print_seq(set.root);
    puts("");
    set_remove(&set, (void *) 4);
    print_seq(set.root);
    puts("");
}

int main() {
    test_insert();
    test_as_array();
    test_set();
    test_merge();
    test_seq();
    puts("PASSED");
}