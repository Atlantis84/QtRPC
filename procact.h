#pragma once
#define PROC_ACTS() \
    PROC_ACT(recipe_size_TV)\
    PROC_ACT(save_recipe_TV)\
    PROC_ACT(check_recipe_trunk_TV)

#define PROC_ACTS_DB() \
    PROC_ACT(recipe_size_TV)\
    PROC_ACT(save_recipe_TV)\
    PROC_ACT(check_recipe_trunk_TV)

#define PROC_ACT(act__) \
    xmlrpc_value* \
    proc_##act__(\
        xmlrpc_env* const envP, \
        xmlrpc_value* const paramArrayP, \
        void* const serverInfo, \
        void* const channelInfo);
PROC_ACTS()

#define PROC_ACT_DB(act__) \
    xmlrpc_value* \
    proc_##act__(\
        xmlrpc_env* const envP, \
        xmlrpc_value* const paramArrayP, \
        void* const serverInfo, \
        void* const channelInfo);
PROC_ACTS_DB()

