#ifndef RPCSERVICETHREAD_H
#define RPCSERVICETHREAD_H

#include <QObject>
#include <QThread>
#include "xmlrpc-c/base.h"
#include "xmlrpc-c/server.h"
#include "xmlrpc-c/server_abyss.h"
#include "xmlrpc-c/base.h"
#include "xmlrpc-c/client.h"
#include "xmlrpc-c/client_global.h"
#include "xmlrpc-c/util.h"
#include "curl/curl.h"
#include "procact.h"
#include <queue>
#include <mutex>
#include <string>
#include <thread>
#define RPCSERVER_NAME			"TVRPCSERVER"
#define RPCSERVER_VERSION		"1.0"

class RPCServiceThread : public QThread
{
    Q_OBJECT
private:
    RPCServiceThread();
private:
    void start_check_upgrade(const char* url, unsigned short port);
    void stop_check_upgrade();
    void notify_upgrade_result();
//    size_t on_write_data(char *ptr, size_t size, size_t nmemb, void *userdata);
    void notify_result(const char* type, const char* line, const char* machine, int result);
//    size_t on_write_result_data(char *ptr, size_t size, size_t nmemb, void *userdata);
    bool startup_db_client(const char* host, unsigned short port);
    void clean_db_client();

    struct upgrade_result {
        char type_[256];
        char line_[256];
        char machine_[256];
        int result_;
    };

    struct check_upgrade_postfields {
        std::string type_;
        std::string fields_;
        std::string buf_;
        uint32_t ts_;
    };

    std::mutex mutex_;
    std::queue<upgrade_result> upgrade_result_;
    std::mutex check_upgrade_mutex_;
    std::deque<check_upgrade_postfields> check_upgrade_postfields_;

    bool stop_check_upgrade_;
    bool check_upgrade_stoped_;
    char center_url_[256];
    unsigned short center_port_;

    static RPCServiceThread* m_pRPCServiceThread;
protected:
    void run();
public:
    static RPCServiceThread* get_instance()
    {
        if(m_pRPCServiceThread == nullptr)
            m_pRPCServiceThread = new RPCServiceThread();
        return m_pRPCServiceThread;
    }

    void check_upgrade_proc();
};

#endif // RPCSERVICETHREAD_H
