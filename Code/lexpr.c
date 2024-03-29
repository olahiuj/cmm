#include "ast.h"
#include "ir.h"
#include "common.h"
#include "type.h"
#include "visitor.h"
#include "symtab.h"

#define RET_TYPE ir_list *
#define ARG list
VISITOR_DEF(AST, lexpr, RET_TYPE);

extern oprd_t oprd_tar();
extern void   oprd_push(oprd_t oprd);
extern void   oprd_pop();

ir_list lexpr_gen(AST_t *node, oprd_t tar) {
    oprd_push(tar);
    ir_list ARG = {0};
    VISITOR_DISPATCH(AST, lexpr, node, &ARG);
    oprd_pop();
    return ARG;
}

VISIT(EXPR_IDEN) {
    ir_list   iden = {0};
    syment_t *sym  = node->sym;
    ASSERT(sym != NULL, "absent sym");

    ir_append(&iden, ir_alloc(IR_ASSIGN, oprd_tar(), sym->var));
    RETURN(iden);
}

VISIT(EXPR_DOT) {
    oprd_t base_var = var_alloc(NULL, node->super.fst_l);

    ir_list base = lexpr_gen(node->base, base_var);
    oprd_t  off  = lit_alloc(node->field->off);

    ir_append(
        &base,
        ir_alloc(IR_BINARY,
                 OP_ADD, oprd_tar(), base_var, off));
    RETURN(base);
}

VISIT(EXPR_ARR) {
    oprd_t  pos     = var_alloc(NULL, node->super.fst_l);
    ir_list arr     = lexpr_gen(node->arr, pos);
    type_t  arr_typ = {0};
    switch (node->arr->kind) {
        case EXPR_IDEN: {
            INSTANCE_OF(node->arr, EXPR_IDEN) {
                arr_typ = cnode->sym->typ;
            }
            break;
        }
        case EXPR_DOT: {
            INSTANCE_OF(node->arr, EXPR_DOT) {
                arr_typ = cnode->typ;
            }
            break;
        }
        default: UNREACHABLE;
    }

    u32 cnt = 0;
    LIST_ITER(node->ind, it) {
        oprd_t  ind_var  = var_alloc(NULL, node->super.fst_l);
        ir_list ind      = ast_gen(it, ind_var);
        oprd_t  acc_size = lit_alloc(arr_typ.acc[cnt++]);
        oprd_t  tmp      = var_alloc(NULL, 0);
        ir_append(&ind,
                  ir_alloc(IR_BINARY,
                           OP_MUL, tmp, ind_var, acc_size));
        ir_append(&ind,
                  ir_alloc(IR_BINARY,
                           OP_ADD, pos, pos, tmp));
        ir_concat(&arr, ind);
    }
    ir_append(&arr, ir_alloc(IR_ASSIGN, oprd_tar(), pos));
    RETURN(arr);
}

VISIT_UNDEF(CONS_PROG);
VISIT_UNDEF(CONS_SPEC);
VISIT_UNDEF(CONS_FUN);

VISIT_UNDEF(DECL_TYP);
VISIT_UNDEF(DECL_VAR);

VISIT_UNDEF(EXPR_CALL);
VISIT_UNDEF(EXPR_ASS);
VISIT_UNDEF(EXPR_INT);
VISIT_UNDEF(EXPR_FLT);
VISIT_UNDEF(EXPR_BIN);
VISIT_UNDEF(EXPR_UNR);

VISIT_UNDEF(STMT_EXPR);
VISIT_UNDEF(STMT_WHLE);
VISIT_UNDEF(STMT_IFTE);
VISIT_UNDEF(STMT_SCOP);
VISIT_UNDEF(STMT_RET);