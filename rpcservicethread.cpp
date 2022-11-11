#include "rpcservicethread.h"
#include "QsLog.h"
#include "reader.h"
#include "writer.h"
#include "GlobDef.h"
#include <queue>
#include <mutex>
#include <string>
#include <thread>
using namespace QsLogging;

RPCServiceThread* RPCServiceThread::m_pRPCServiceThread = nullptr;
static xmlrpc_env env;
static xmlrpc_server_abyss_t* serverP = nullptr;
static char db_server_url[256] = {0};
size_t on_write_result_data(char *ptr, size_t size, size_t nmemb, void *userdata)
{
    char* buffer = (char*)userdata;
    int len = (int)strlen(buffer);
    memcpy_s(buffer + len, 4096 - len, ptr, size * nmemb);

    return size * nmemb;
}

size_t on_write_data(char *ptr, size_t size, size_t nmemb, void *userdata)
{
    std::string* buffer = (std::string*)userdata;
    buffer->append(ptr, size * nmemb);

    return size * nmemb;
}

xmlrpc_value* proc_check_recipe_trunk_TV(
    xmlrpc_env* const envP,
    xmlrpc_value* const paramArrayP,
    void* const serverInfo,
    void* const channelInfo)
{
    CleanServerEnv();
    CleanClientEnv();

    xmlrpc_value* dataP = xmlrpc_struct_new(envP);
    xmlrpc_value* arrayP;
    xmlrpc_decompose_value(envP, paramArrayP, "(A)", &arrayP);
    CheckIfFaultOccurredServer();

    if (xmlrpc_value_type(arrayP) != XMLRPC_TYPE_ARRAY || !xmlrpc_array_size(envP, arrayP)) {
        return build_ret_value(EC_INVALID_ARGS, dataP);
    }
    call_db_server(check_recipe_trunk_TV_DB);
    decompose_ret_value();
    return build_ret_value(ec, dataP);
}

xmlrpc_value*
proc_recipe_size_TV(
    xmlrpc_env* const envP,
    xmlrpc_value* const paramArrayP,
    void* const serverInfo,
    void* const channelInfo)
{
    CleanServerEnv();
    CleanClientEnv();

    xmlrpc_value* dataP = xmlrpc_struct_new(envP);

    const char* name;
    const char* line;
    const char* deviceno;
    int size;
    int device_len;
    xmlrpc_decompose_value(envP, paramArrayP, "({s:s,s:i,s:s,s:6,*})", "name", &name, "size", &size, "line", &line, "deviceno", &deviceno,&device_len);
    CheckIfFaultOccurredServer();

    call_db_server(recipe_size_TV_DB);
    decompose_ret_value();
    return build_ret_value(ec, dataP);
}

xmlrpc_value*
proc_save_recipe_TV(
    xmlrpc_env* const envP,
    xmlrpc_value* const paramArrayP,
    void* const serverInfo,
    void* const channelInfo)
{
    CleanServerEnv();
    CleanClientEnv();

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

    call_db_server(save_recipe_TV_DB);
    decompose_ret_value();
    return build_ret_value(ec, dataP);
}

RPCServiceThread::RPCServiceThread()
{
    center_url_[256] = { 0 };
    center_port_ = 0;
    stop_check_upgrade_ = false;
    check_upgrade_stoped_ = false;
}

void RPCServiceThread::start_check_upgrade(const char* url, unsigned short port)
{

//    std::shared_ptr<RPCServiceThread> rpcThrd = std::make_shared<RPCServiceThread>();
//    std::function<void()> addFunc = std::bind(&RPCServiceThread::check_upgrade_proc,rpcThrd);
    strcpy_s(center_url_, url);
    center_port_ = port;

    stop_check_upgrade_ = check_upgrade_stoped_ = false;
    std::thread thr(&RPCServiceThread::check_upgrade_proc,RPCServiceThread::get_instance());
    thr.detach();

    std::thread thr1(&RPCServiceThread::notify_upgrade_result,RPCServiceThread::get_instance());
    thr1.detach();
}

void RPCServiceThread::check_upgrade_proc()
{
    static xmlrpc_env env;
    /* Initialize our error-handling environment. */
    xmlrpc_env_init(&env);

    std::string buffer;

    CURL* curl = curl_easy_init();

    curl_easy_setopt(curl, CURLOPT_URL, center_url_);
    curl_easy_setopt(curl, CURLOPT_PORT, center_port_);
    curl_easy_setopt(curl, CURLOPT_HEADER, 0);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, on_write_data);
    curl_easy_setopt(curl, CURLOPT_POST, 1);
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 0);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);

    for (;;) {
        if (stop_check_upgrade_) {
            break;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(30));

        std::lock_guard<std::mutex> m(check_upgrade_mutex_);
        if (check_upgrade_postfields_.empty()) {
            continue;
        }

        auto postfields = check_upgrade_postfields_.back();
        check_upgrade_postfields_.pop_back();
        do {
            if (GetTickCount() - postfields.ts_ < 5 * 1000) {
                break;
            }
            postfields.ts_ = GetTickCount();

            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postfields.fields_.c_str());
            curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, postfields.fields_.size());

            buffer.clear();
            if (curl_easy_perform(curl) != CURLE_OK) {
                break;
            }
            buffer.append("\0");

            // 如果没有换产，中控不推送数据
            if (buffer.empty()) {
                break;
            }

// 			if (buffer == postfields.buf_) {
// 				break;
// 			}

            Json::Value root;
            Json::Reader reader;
            if (!reader.parse(buffer.c_str(), root)) {
                break;
            }

            if (root["code"].asString() != "OK") {
                break;
            }

            xmlrpc_value* array = xmlrpc_array_new(&env);

            Json::Value& data = root["data"];
            Json::Value& type = root["requestType"];
            if (!type.isString()) {
                break;
            }

            CleanClientEnv();

            const char* typeName = type.asCString();
            for (int i = 0; i < (int)data.size(); i++) {
                Json::Value& ctx = data[i];
                const char* areaName = ctx["areaName"].asCString();
                const char* machineName = ctx["machineName"].asCString();
                const char* boardStyle = ctx["boardStyle"].asCString();
                const char* machineStyleName = ctx["machineStyleName"].asCString();
                const char* halfMaterialNumber = ctx["halfMaterialNumber"].asCString();

                xmlrpc_value* resultP = xmlrpc_client_call(&env, db_server_url, "proc.tcon_testplan_exist", "({s:s,s:s,s:s})", "boardStyle", boardStyle, "machineStyleName", machineStyleName, "halfMaterialNumber", halfMaterialNumber);
                if (env.fault_occurred) {
//					L_TRACE(env.fault_string);
                    notify_result(typeName, areaName, machineName, -1);
                    continue;
                }
                int ec;
                xmlrpc_decompose_value(&env, resultP, "{s:i,*}", "ec", &ec);
                xmlrpc_DECREF(resultP);

//				L_TRACE("fault_cde: {0}: size: {1}", env.fault_code, ec);
                if (env.fault_occurred || ec <= EC_SUCCESS) {
                    notify_result(typeName, areaName, machineName, -1);
                    continue;
                }

//				L_TRACE("[{0}] = machineName: {1}, boardStyle: {2}, machineStyleName: {3}, halfMaterialNumber: {4}", i, machineName, boardStyle, machineStyleName, halfMaterialNumber);
                xmlrpc_value* d = xmlrpc_struct_new(&env);
                xmlrpc_struct_set_value(&env, d, "areaName", xmlrpc_string_new(&env, areaName));
                xmlrpc_struct_set_value(&env, d, "machineName", xmlrpc_base64_new(&env, strlen(machineName), (unsigned char*)machineName));
                xmlrpc_struct_set_value(&env, d, "boardStyle", xmlrpc_string_new(&env, boardStyle));
                xmlrpc_struct_set_value(&env, d, "machineStyleName", xmlrpc_string_new(&env, machineStyleName));
                xmlrpc_struct_set_value(&env, d, "halfMaterialNumber", xmlrpc_string_new(&env, halfMaterialNumber));
                xmlrpc_struct_set_value(&env, d, "stat", xmlrpc_int_new(&env, 1));

                xmlrpc_array_append_item(&env, array, d);
            }

            if (env.fault_occurred) {
                break;
            }

            xmlrpc_value* resultP = xmlrpc_client_call(&env, db_server_url, "proc.update_recipe", "({s:s,s:i,s:A})", "testtype", typeName, "size", xmlrpc_array_size(&env, array), "data", array);
            xmlrpc_DECREF(array);
            if (env.fault_occurred) {
                continue;
            }

            xmlrpc_value* ecP;
            xmlrpc_struct_read_value(&env, resultP, "ec", &ecP);
            xmlrpc_DECREF(resultP);
            if (env.fault_occurred) {
                break;
            }
            int ec;
            xmlrpc_read_int(&env, ecP, &ec);
            if (env.fault_occurred || ec < 0) {
                break;
            }

            postfields.buf_ = buffer;
        } while (0);

        check_upgrade_postfields_.push_front(postfields);
    }

    check_upgrade_stoped_ = true;

    curl_easy_cleanup(curl);
}

void RPCServiceThread::notify_result(const char* type, const char* line, const char* machine, int result)
{
    std::lock_guard<std::mutex> lock(mutex_);
    upgrade_result up_result = { 0 };
    const char* p = strrchr(type, '_');
    if (!p) { // 非法请求
        return;
    }

    strncpy_s(up_result.type_, type, (p + 1) - type);
    strcpy_s(up_result.line_, line);
    strcpy_s(up_result.machine_, machine);
    up_result.result_ = result;

    upgrade_result_.push(up_result);
}

void RPCServiceThread::notify_upgrade_result()
{
    CURL* curl = curl_easy_init();
    char buffer[4096] = { 0 };

// 	curl_easy_setopt(curl, CURLOPT_URL, "http://192.168.8.131");
// 	curl_easy_setopt(curl, CURLOPT_PORT, 8080);
    curl_easy_setopt(curl, CURLOPT_URL, center_url_);
    curl_easy_setopt(curl, CURLOPT_PORT, center_port_);
    curl_easy_setopt(curl, CURLOPT_HEADER, 0);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, buffer);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, on_write_result_data);
    curl_easy_setopt(curl, CURLOPT_POST, 1);
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 0);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);

    Json::Reader reader;
    for (; !check_upgrade_stoped_;) {
        std::this_thread::sleep_for(std::chrono::milliseconds(300));

        std::lock_guard<std::mutex> lock(mutex_);
        if (upgrade_result_.empty()) {
            continue;
        }

        upgrade_result result = upgrade_result_.front();
        upgrade_result_.pop();

        char reType[200] = { 0 };
        Json::Value root;
        if (result.result_ != EC_SUCCESS) {
            sprintf_s(reType, "%s%s", result.type_, "NULL");
        }
        else {
            sprintf_s(reType, "%s%s", result.type_, "RESULT");
        }
        root["reType"] = reType;
        root["lineName"] = result.line_;
        root["machineName"] = result.machine_;
// 		root["result"] = result.result_;

        Json::FastWriter writer;
        std::string postfields = writer.write(root);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postfields.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, postfields.size());

//		L_TRACE(postfields.c_str());
        memset(buffer, 0, sizeof(buffer));
        if (curl_easy_perform(curl) != CURLE_OK) {
            continue;
        }

//		L_DEBUG(buffer);
        if (!reader.parse(buffer, root)) {
            upgrade_result_.push(result);
            continue;
        }

        if (!root["code"].isString()) {
            upgrade_result_.push(result);
            continue;
        }
    }

    curl_easy_cleanup(curl);
}

void RPCServiceThread::stop_check_upgrade()
{
    stop_check_upgrade_ = true;

    while (!check_upgrade_stoped_) {
        std::this_thread::sleep_for(std::chrono::milliseconds(30));
    }
}

bool RPCServiceThread::startup_db_client(const char *host, unsigned short port)
{
    /* Initialize our error-handling environment. */
    xmlrpc_env_init(&env);
    if (env.fault_occurred) {
        QLOG_ERROR()<<env.fault_string<<env.fault_code;
        return false;
    }

    /* Create the global XML-RPC client object. */
    xmlrpc_client_init2(&env, XMLRPC_CLIENT_NO_FLAGS, RPCSERVER_NAME, RPCSERVER_VERSION, NULL, 0);
    if (env.fault_occurred) {
        QLOG_ERROR()<<env.fault_string<<env.fault_code;
        return false;
    }

    sprintf_s(db_server_url, sizeof(db_server_url) / sizeof(db_server_url[0]), "http://%s:%d/RPC2", host, port);

    QLOG_INFO()<<"[DB] addr:"<<db_server_url;

    return true;
}

void RPCServiceThread::run()
{
    if(curl_global_init(CURL_GLOBAL_ALL) == CURLE_OK)
        QLOG_INFO()<<"curl init SUCCESS";
    else
        QLOG_WARN()<<"curl init FAILED";
    xmlrpc_server_abyss_parms parms = { 0 };
    xmlrpc_registry* registryP = nullptr;

    do
    {
        xmlrpc_env_init(&env);
        registryP = xmlrpc_registry_new(&env);
        if (env.fault_occurred) {
            QLOG_WARN()<<"xmlrpc_registry_new() failed. "<<env.fault_string;
            break;
        }

#define PROC_ACT(act__) {"proc."#act__, &proc_##act__ },
        static xmlrpc_method_info3 methodInfo[] = {
            PROC_ACTS()
        };
#undef PROC_ACT
        for (unsigned int i = 0; i < sizeof(methodInfo) / sizeof(methodInfo[0]); i++) {
                    xmlrpc_registry_add_method3(&env, registryP, &methodInfo[i]);
                    if (env.fault_occurred) {
                        QLOG_WARN()<<"xmlrpc_registry_add_method3() failed. "<<methodInfo[i].methodName;
                        break;
                    }
                }

                if (env.fault_occurred) {
                    QLOG_ERROR()<<"xmlrpc_registry_add_method3() failed. "<<env.fault_string;
                    break;
                }

                char szLogName[MAX_PATH] = { 0 };
                parms.config_file_name = nullptr;   /* Select the modern normal API */
                parms.registryP = registryP;
                parms.port_number = 8880;
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
                    if (!startup_db_client("127.0.0.1", 8997)) {
                        break;
                    }

                    start_check_upgrade("http://127.0.0.1", 8080);
                    QLOG_WARN()<<"Running server @8080";
                    xmlrpc_server_abyss_run_server(&env, serverP);
                    QLOG_WARN()<<"Stopping server @8080";
                    xmlrpc_server_abyss_restore_sig(oldHandlersP);

                    free(oldHandlersP);

                    stop_check_upgrade();
                }
                xmlrpc_server_abyss_destroy(serverP);

                if (env.fault_occurred) {
                    QLOG_WARN()<<"xmlrpc_server_abyss() failed. "<<env.fault_string;
                    break;
                }
    }while(0);

    if (registryP)
    {
        xmlrpc_registry_free(registryP);
    }

    xmlrpc_server_abyss_global_term();

    clean_db_client();
    curl_global_cleanup();
}

void RPCServiceThread::clean_db_client()
{
    /* Clean up our error-handling environment. */
    xmlrpc_env_clean(&env);

    /* Shutdown our XML-RPC client library. */
    xmlrpc_client_cleanup();
}

