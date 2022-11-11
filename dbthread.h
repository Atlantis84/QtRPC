#ifndef DBTHREAD_H
#define DBTHREAD_H

#include <QObject>
#include <QThread>
class DBThread : public QThread
{
    Q_OBJECT
public:
    DBThread();
protected:
    void run();
};

#endif // DBTHREAD_H
