#include <extensionsystem/iplugin.h>
#include <extensionsystem/pluginmanager.h>
#include <extensionsystem/pluginspec.h>
#include "../plugins/helloworld/helloservice.h"

#include <QCoreApplication>
#include <QFileInfo>

using namespace ExtensionSystem;

enum { OptionIndent = 4, DescriptionIndent = 34 };

const char corePluginNameC[] = "Core";

const char fixedOptionsC[] =
        " [OPTION]... [FILE]...\n"
        "Options:\n"
        "    -help                         Display this help\n"
        "    -version                      Display program version\n"
        "    -client                       Attempt to connect to already running first instance\n"
        "    -settingspath <path>          Override the default path where user settings are stored\n"
        "    -installsettingspath <path>   Override the default path from where user-independent settings are read\n"
        "    -pid <pid>                    Attempt to connect to instance given by pid\n"
        "    -block                        Block until editor is closed\n"
        "    -pluginpath <path>            Add a custom search path for plugins\n";

const char HELP_OPTION1[] = "-h";
const char HELP_OPTION2[] = "-help";
const char HELP_OPTION3[] = "/h";
const char HELP_OPTION4[] = "--help";
const char VERSION_OPTION[] = "-version";

static void printVersion(const PluginSpec *coreplugin)
{
    QString version;
    QTextStream str(&version);
    str << '\n' << "QES" << ' ' << coreplugin->version()<< " based on Qt " << qVersion() << "\n\n";
    PluginManager::formatPluginVersions(str);
    str << '\n' << coreplugin->copyright() << '\n';

    qWarning("%s", qPrintable(version));
}

static void printHelp(const QString &a0)
{
    QString help;
    QTextStream str(&help);
    str << "Usage: " << a0 << fixedOptionsC;
    PluginManager::formatOptions(str, OptionIndent, DescriptionIndent);
    PluginManager::formatPluginOptions(str, OptionIndent, DescriptionIndent);

    qWarning("%s", qPrintable(help));
}

static void printSpecs(QList<PluginSpec *> plugins)
{
    qDebug() << "==================== get specs started ====================";

    for (int i = 0; i < plugins.count(); ++i) {
        PluginSpec *spec = plugins[i];

        if (i != 0)
            qDebug() << "########################################";

        qDebug() << "name:" << spec->name();
        qDebug() << "version:" << spec->version();
        qDebug() << "compatVersion:" << spec->compatVersion();
        qDebug() << "vendor:" << spec->vendor();
        qDebug() << "copyright:" << spec->copyright();
        qDebug() << "license:" << spec->license();
        qDebug() << "description:" << spec->description();
        qDebug() << "url:" << spec->url();
        qDebug() << "category:" << spec->category();

        QVector<PluginDependency> vector = spec->dependencies();
        foreach (PluginDependency dep, vector) {
            qDebug() << "Depends:" << dep.toString();
        }

        qDebug() << "platformSpecification:" << spec->platformSpecification();
        qDebug() << "errorString:" << spec->errorString();
    }

    qDebug() << "==================== get specs finished ====================";
}

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);

    app.setAttribute(Qt::AA_UseHighDpiPixmaps);

    const QStringList pluginArguments = app.arguments();

    QString pluginPath = QCoreApplication::applicationDirPath() + "/plugins/";

    PluginManager pluginManager;
    PluginManager::setPluginIID(QLatin1String("waleon.blog.csdn.net.qesplugin"));
    PluginManager::setPluginPaths(QStringList() << pluginPath);

    QMap<QString, QString> foundAppOptions;
    if (pluginArguments.size() > 1) {
        QMap<QString, bool> appOptions;
        appOptions.insert(QLatin1String(HELP_OPTION1), false);
        appOptions.insert(QLatin1String(HELP_OPTION2), false);
        appOptions.insert(QLatin1String(HELP_OPTION3), false);
        appOptions.insert(QLatin1String(HELP_OPTION4), false);
        appOptions.insert(QLatin1String(VERSION_OPTION), false);
        QString errorMessage;
        if (!PluginManager::parseOptions(pluginArguments, appOptions, &foundAppOptions, &errorMessage)) {
            qCritical() << errorMessage;
            return -1;
        }
    }

    const QList<PluginSpec *> plugins = PluginManager::plugins();
    PluginSpec *coreplugin = nullptr;

    printSpecs(plugins);

    foreach (PluginSpec *spec, plugins) {
        if (spec->name() == QLatin1String(corePluginNameC)) {
            coreplugin = spec;
            break;
        }
    }

    if (coreplugin == nullptr) {
        qCritical() << "Could not find Core plugin.";
        return 1;
    }

    if (!coreplugin->isEffectivelyEnabled()) {
        qCritical() << "Core plugin is disabled.";
        return 1;
    }

    if (coreplugin->hasError()) {
        qCritical() << coreplugin->errorString();
        return 1;
    }

    if (foundAppOptions.contains(QLatin1String(VERSION_OPTION))) {
        printVersion(coreplugin);
        return 0;
    }

    if (foundAppOptions.contains(QLatin1String(HELP_OPTION1))
            || foundAppOptions.contains(QLatin1String(HELP_OPTION2))
            || foundAppOptions.contains(QLatin1String(HELP_OPTION3))
            || foundAppOptions.contains(QLatin1String(HELP_OPTION4))) {
        printHelp(QFileInfo(app.applicationFilePath()).baseName());
        return 0;
    }

    PluginManager::loadPlugins();
    if (coreplugin->hasError()) {
        qWarning() << "Failed to load core:" << coreplugin->errorString();
        return 1;
    }

    Hello::Service *helloService = PluginManager::getObject<Hello::Service>();
    if (helloService != nullptr)
        helloService->sayHello();


//    // Set up remote arguments.
//    QObject::connect(&app, &SharedTools::QtSingleApplication::messageReceived,
//                     &pluginManager, &PluginManager::remoteArguments);

//    QObject::connect(&app, SIGNAL(fileOpenRequest(QString)), coreplugin->plugin(),
//                     SLOT(fileOpenRequest(QString)));

    // shutdown plugin manager on the exit
    QObject::connect(&app, &QCoreApplication::aboutToQuit, &pluginManager, &PluginManager::shutdown);

    return app.exec();
}
