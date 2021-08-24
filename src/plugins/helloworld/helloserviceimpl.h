#ifndef HELLO_SERVICE_IMPL_H
#define HELLO_SERVICE_IMPL_H

#include "helloservice.h"

namespace Hello {

class HelloServiceImplPrivate;

class HelloServiceImpl : public QObject, public Hello::Service
{
    Q_OBJECT

    Q_INTERFACES(Hello::Service)

public:
    explicit HelloServiceImpl(QObject *parent = Q_NULLPTR);
    ~HelloServiceImpl();

    void sayHello() Q_DECL_OVERRIDE;
};

} // namespace Hello

#endif // HELLO_SERVICE_IMPL_H
