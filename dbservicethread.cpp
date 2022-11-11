#include "dbservicethread.h"
#include "QsLog.h"
#include "reader.h"
#include "writer.h"
#include "GlobDef.h"
#include <queue>
#include <mutex>
#include <string>
#include <thread>
using namespace QsLogging;
static xmlrpc_env env;
static xmlrpc_server_abyss_t* serverP = 0;
DBServiceThread* DBServiceThread::m_pDBServiceThread=nullptr;
xmlrpc_value*
proc_check_recipe_trunk_TV_DB(
    xmlrpc_env* const envP,
    xmlrpc_value* const paramArrayP,
    void* const serverInfo,
    void* const channelInfo)
{
    CleanServerEnv();

    xmlrpc_value* dataP = xmlrpc_struct_new(envP);
    xmlrpc_value* arrayP;
    xmlrpc_decompose_value(envP, paramArrayP, "(A)", &arrayP);
    CheckIfFaultOccurredServer();

    std::vector<std::string> md5;
    for (int i = 0; i < xmlrpc_array_size(envP, arrayP); i++) {
        xmlrpc_value* m = xmlrpc_array_get_item(envP, arrayP, i);
        const char* md5_str;
        xmlrpc_read_string(envP, m, &md5_str);
        md5.push_back(md5_str);
    }
    xmlrpc_DECREF(arrayP);

    xmlrpc_value* valueP = xmlrpc_array_new(envP);
    auto cb = [&envP, &valueP](const std::string& md5)
    {
        xmlrpc_array_append_item(envP, valueP, xmlrpc_string_new(envP, md5.c_str()));
    };
    xmlrpc_int32 ec /*= mysqlx_session_->check_tv_hstvt_trunk(md5, cb)*/;

    xmlrpc_struct_set_value(envP, dataP, "md5", valueP);
    xmlrpc_DECREF(valueP);

    return build_ret_value(ec, dataP);
}

xmlrpc_value*
proc_recipe_size_TV_DB(
    xmlrpc_env* const envP,
    xmlrpc_value* const paramArrayP,
    void* const serverInfo,
    void* const channelInfo)
{
    CleanServerEnv();

    xmlrpc_value* dataP = xmlrpc_struct_new(envP);

    const char* name;
    int size;
    const char* line;
    const char* deviceno;
    int device_len;
    xmlrpc_decompose_value(envP, paramArrayP, "({s:s,s:i,s:s,s:6,*})", "name", &name, "size", &size, "line", &line, "deviceno", &deviceno,&device_len);

    CheckIfFaultOccurredServer();

    int32_t ec /*= mysqlx_session_->add_tv_recipe_size(name, size,line,deviceno)*/;

    return build_ret_value(ec, dataP);
}

xmlrpc_value*
proc_save_recipe_TV_DB(
    xmlrpc_env* const envP,
    xmlrpc_value* const paramArrayP,
    void* const serverInfo,
    void* const channelInfo)
{
    CleanServerEnv();

    xmlrpc_value* dataP = xmlrpc_struct_new(envP);
    const char* name;
    const char* line;
    const char* deviceno;
    const char* md5_str;
    const char* ctx;
    int offset;
    int size;
    int device_len;
    xmlrpc_decompose_value(envP, paramArrayP, "({s:s,s:s,s:6,s:s,s:s,s:i,s:i,*})", "name", &name, "line", &line, "deviceno", &deviceno,&device_len, "md5", &md5_str, "data", &ctx, "offset", &offset, "size", &size);

    CheckIfFaultOccurredServer();

    int ec /*= mysqlx_session_->add_tv_testplan(name,md5_str, ctx, offset, size, "")*/;

    return build_ret_value(ec, dataP);
}

DBServiceThread::DBServiceThread()
{

}

void DBServiceThread::run()
{
    xmlrpc_server_abyss_parms parms = { 0 };
    xmlrpc_registry* registryP = 0;

    do {
        xmlrpc_env_init(&env);

        registryP = xmlrpc_registry_new(&env);
        if (env.fault_occurred) {
            QLOG_ERROR()<<"xmlrpc_registry_new() failed."<<env.fault_string;
            break;
        }

#define PROC_ACT(act__) {"proc."###act__, &proc_##act__ },
        static xmlrpc_method_info3 methodInfo[] = {
            PROC_ACTS()
        };
#undef PROC_ACT

        for (int i = 0; i < sizeof(methodInfo) / sizeof(methodInfo[0]); i++) {
            xmlrpc_registry_add_method3(&env, registryP, &methodInfo[i]);
            if (env.fault_occurred) {
                QLOG_ERROR()<<"xmlrpc_registry_add_method3() failed."<<methodInfo[i].methodName;
                break;
            }
        }

        if (env.fault_occurred) {
            QLOG_ERROR()<<"xmlrpc_registry_add_method3() failed."<<env.fault_string;
            break;
        }

        char szLogName[MAX_PATH] = { 0 };

        memset(&parms, 0, sizeof(parms));
        parms.config_file_name = NULL;   /* Select the modern normal API */
        parms.registryP = registryP;
        parms.port_number = 8997;
        parms.log_file_name = szLogName;

        xmlrpc_server_abyss_global_init(&env);
        if (env.fault_occurred) {
            QLOG_ERROR()<<"xmlrpc_server_abyss_global_init() failed."<<env.fault_string;
            break;
        }

        xmlrpc_server_abyss_create(&env, &parms, XMLRPC_APSIZE(log_file_name), &serverP);
        if (env.fault_occurred) {
            QLOG_ERROR()<<"xmlrpc_server_abyss_create() failed."<<env.fault_string;
            break;
        }
        xmlrpc_server_abyss_sig* oldHandlersP;

        xmlrpc_server_abyss_setup_sig(&env, serverP, &oldHandlersP);

        if (!env.fault_occurred) {
            QLOG_WARN()<<"Running server @"<<QString::number(8997);
            xmlrpc_server_abyss_run_server(&env, serverP);

            QLOG_WARN()<<"Stopping server @"<<QString::number(8997);
            xmlrpc_server_abyss_restore_sig(oldHandlersP);

            free(oldHandlersP);
        }
        xmlrpc_server_abyss_destroy(serverP);

        if (env.fault_occurred) {
            QLOG_ERROR()<<"xmlrpc_server_abyss() failed."<<env.fault_string;
            break;
        }
    } while (0);

    if (registryP) {
        xmlrpc_registry_free(registryP);
    }

    xmlrpc_server_abyss_global_term();
}
