#include "helloplugin.h"
#include "helloserviceimpl.h"
#include "extensionsystem/pluginmanager.h"

namespace Hello {
namespace Internal {

class HelloPluginPrivate
{
public:
    HelloPluginPrivate();
    ~HelloPluginPrivate();

    HelloServiceImpl helloService;
};

HelloPluginPrivate::HelloPluginPrivate()
{
    ExtensionSystem::PluginManager::addObject(&helloService);
}

HelloPluginPrivate::~HelloPluginPrivate()
{
    ExtensionSystem::PluginManager::removeObject(&helloService);
}

static HelloPluginPrivate *dd = nullptr;

HelloPlugin::HelloPlugin()
{
}

HelloPlugin::~HelloPlugin()
{
    delete dd;
}

bool HelloPlugin::initialize(const QStringList &arguments, QString *errorMessage)
{
    Q_UNUSED(arguments)
    Q_UNUSED(errorMessage)

    dd = new HelloPluginPrivate();

    qDebug() << "initialize Hello Plugin";

    return true;
}

void HelloPlugin::extensionsInitialized()
{
    qDebug() << "extensions initialized Hello Plugin";
}

bool HelloPlugin::delayedInitialize()
{
    qDebug() << "delayed initialize Hello Plugin";

    return true;
}

} // namespace Internal
} // namespace Hello
