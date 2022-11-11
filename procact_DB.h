#pragma once
#define PROC_ACTS() \
    PROC_ACT(recipe_size_TV_DB)\
    PROC_ACT(save_recipe_TV_DB)\
    PROC_ACT(check_recipe_trunk_TV_DB)

#define PROC_ACT(act__) \
    xmlrpc_value* \
    proc_##act__(\
        xmlrpc_env* const envP, \
        xmlrpc_value* const paramArrayP, \
        void* const serverInfo, \
        void* const channelInfo);
PROC_ACTS()

