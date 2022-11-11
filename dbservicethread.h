#ifndef DBSERVICETHREAD_H
#define DBSERVICETHREAD_H
#include <QObject>
#include <QThread>
#include "xmlrpc-c/base.h"
#include "xmlrpc-c/server.h"
#include "xmlrpc-c/server_abyss.h"
#include "curl/curl.h"
#include "procact_DB.h"
class DBServiceThread : public QThread
{
    Q_OBJECT
private:
    DBServiceThread();

    static DBServiceThread* m_pDBServiceThread;
protected:
    void run();

public:
    static DBServiceThread* get_instance()
    {
        if(m_pDBServiceThread == nullptr)
            m_pDBServiceThread = new DBServiceThread();
        return m_pDBServiceThread;
    }
};

#endif // DBSERVICETHREAD_H
