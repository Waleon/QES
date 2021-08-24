#ifndef HELLO_PLUGIN_H
#define HELLO_PLUGIN_H

#include <extensionsystem/iplugin.h>

namespace Hello {
namespace Internal {

class HelloPluginPrivate;

class HelloPlugin : public ExtensionSystem::IPlugin
{
    Q_OBJECT

    Q_PLUGIN_METADATA(IID "waleon.blog.csdn.net.qesplugin" FILE "hello.json")

public:
    HelloPlugin();
    ~HelloPlugin();

    bool initialize(const QStringList &arguments, QString *errorMessage);
    void extensionsInitialized();
    bool delayedInitialize();

private:
    QScopedPointer<Internal::HelloPluginPrivate> dd_ptr;
    Q_DECLARE_PRIVATE_D(dd_ptr,Internal::HelloPlugin)
};

} // namespace Internal
} // namespace Hello

#endif // HELLO_PLUGIN_H
