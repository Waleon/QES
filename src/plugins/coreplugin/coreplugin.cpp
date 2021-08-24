#include "coreplugin.h"

namespace Core {
namespace Internal {

CorePlugin::CorePlugin()
{
}

CorePlugin::~CorePlugin()
{

}

bool CorePlugin::initialize(const QStringList &arguments, QString *errorMessage)
{
    Q_UNUSED(arguments)
    Q_UNUSED(errorMessage)

    qDebug() << "initialize Core Plugin";

    return true;
}

void CorePlugin::extensionsInitialized()
{
    qDebug() << "extensions initialized Core Plugin";
}

bool CorePlugin::delayedInitialize()
{
    qDebug() << "delayed initialize Core Plugin";

    return true;
}

} // namespace Internal
} // namespace Core
