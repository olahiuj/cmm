#pragma once
#include "cfg.h"
#include "ir.h"

#define LOCAL_OPT(F) \
    F(lvn)

#define CLEANUP_OPT(F) \
    F(cp_rewrite)      \
    F(dce)

#define ONCE_OPT(F) \
    F(copy_rewrite) \
    F(licm)

#define OPT_REGISTER(OPT) extern void do_##OPT(cfg_t *cfg);
#define OPT_EXECUTE(OPT) do_##OPT(cfg);

void optimize(cfg_t *cfg);