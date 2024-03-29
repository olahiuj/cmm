#include "opt.h"
#include "cfg.h"

#define NROUND 5

CLEANUP_OPT(OPT_REGISTER)
LOCAL_OPT(OPT_REGISTER)
ONCE_OPT(OPT_REGISTER)

void optimize(cfg_t *cfg) {
    LOG("optimize %s", cfg->str);
    LOCAL_OPT(OPT_EXECUTE)
    CLEANUP_OPT(OPT_EXECUTE)
    ONCE_OPT(OPT_EXECUTE)
    LOCAL_OPT(OPT_EXECUTE)
    CLEANUP_OPT(OPT_EXECUTE)
}