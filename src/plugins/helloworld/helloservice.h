#ifndef HELLO_SERVICE_H
#define HELLO_SERVICE_H

#include <QObject>

namespace Hello {

class Service
{
public:
    virtual ~Service() {}

    virtual void sayHello() = 0;
};

} // namespace Hello

#define Hello_Service_iid "waleon.blog.csdn.net.qesplugin.hello.service"
Q_DECLARE_INTERFACE(Hello::Service, Hello_Service_iid)

#endif // HELLO_SERVICE_H
