#ifndef CORE_PLUGIN_H
#define CORE_PLUGIN_H

#include <extensionsystem/iplugin.h>

namespace Core {
namespace Internal {

class CorePlugin : public ExtensionSystem::IPlugin
{
    Q_OBJECT

    Q_PLUGIN_METADATA(IID "waleon.blog.csdn.net.qesplugin" FILE "coreplugin.json")

public:
    CorePlugin();
    ~CorePlugin();

    bool initialize(const QStringList &arguments, QString *errorMessage);
    void extensionsInitialized();
    bool delayedInitialize();
};

} // namespace Internal
} // namespace Core

#endif // CORE_PLUGIN_H
