#include "helloserviceimpl.h"
#include <QDebug>

using namespace Hello;

HelloServiceImpl::HelloServiceImpl(QObject *parent)
    : QObject(parent)
{
}

HelloServiceImpl::~HelloServiceImpl()
{
}

void HelloServiceImpl::sayHello()
{
    qWarning() << "Hello, World!";
}
