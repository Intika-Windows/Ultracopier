#include "listener.h"

Listener::Listener()
{
    server.setName(tr("Ultracopier"));
    connect(&server,&ServerCatchcopy::newCopyWithoutDestination,		this,&Listener::copyWithoutDestination);
    connect(&server,&ServerCatchcopy::newCopy,                          this,&Listener::copy);
    connect(&server,&ServerCatchcopy::newMoveWithoutDestination,		this,&Listener::moveWithoutDestination);
    connect(&server,&ServerCatchcopy::newMove,                          this,&Listener::move);
    connect(&server,&ServerCatchcopy::error,                            this,&Listener::error);
    connect(&server,&ServerCatchcopy::clientName,                       this,&Listener::clientName);
}

void Listener::listen()
{
    ULTRACOPIER_DEBUGCONSOLE(Ultracopier::DebugLevel_Notice,"start");
    if(server.listen())
        emit newState(Ultracopier::FullListening);
    else
        emit newState(Ultracopier::NotListening);
}

void Listener::close()
{
    ULTRACOPIER_DEBUGCONSOLE(Ultracopier::DebugLevel_Notice,"start");
    server.close();
    emit newState(Ultracopier::NotListening);
}

const QString Listener::errorString()
{
    ULTRACOPIER_DEBUGCONSOLE(Ultracopier::DebugLevel_Notice,"start");
    return server.errorString();
}

void Listener::setResources(OptionInterface * options,const QString &writePath,const QString &pluginPath,const bool &portableVersion)
{
    Q_UNUSED(options);
    Q_UNUSED(writePath);
    Q_UNUSED(pluginPath);
    Q_UNUSED(portableVersion);
}

/// \brief to get the options widget, NULL if not have
QWidget * Listener::options()
{
    return NULL;
}

void Listener::transferFinished(const quint32 &orderId,const bool &withError)
{
    ULTRACOPIER_DEBUGCONSOLE(Ultracopier::DebugLevel_Notice,"start, orderId: "+QString::number(orderId)+", withError: "+QString::number(withError));
    server.copyFinished(orderId,withError);
}

void Listener::transferCanceled(const quint32 &orderId)
{
    ULTRACOPIER_DEBUGCONSOLE(Ultracopier::DebugLevel_Notice,"start, orderId: "+QString::number(orderId));
    server.copyCanceled(orderId);
}

/// \brief to reload the translation, because the new language have been loaded
void Listener::newLanguageLoaded()
{
}

void Listener::error(QString error)
{
    Q_UNUSED(error);
    ULTRACOPIER_DEBUGCONSOLE(Ultracopier::DebugLevel_Warning,"warning emited from Catchcopy lib: "+error);
}

void Listener::clientName(quint32 client,QString name)
{
    Q_UNUSED(client);
    Q_UNUSED(name);
    ULTRACOPIER_DEBUGCONSOLE(Ultracopier::DebugLevel_Information,QString("clientName: %1, for the id: %2").arg(name).arg(client));
}

void Listener::copyWithoutDestination(const quint32 &orderId,const QStringList &sources)
{
    ULTRACOPIER_DEBUGCONSOLE(Ultracopier::DebugLevel_Information,QString("copyWithoutDestination(%1,%2)").arg(orderId).arg(sources.join(";")));
    emit newCopyWithoutDestination(orderId,sources);
}

void Listener::copy(const quint32 &orderId,const QStringList &sources,const QString &destination)
{
    ULTRACOPIER_DEBUGCONSOLE(Ultracopier::DebugLevel_Information,QString("copy(%1,%2,%3)").arg(orderId).arg(sources.join(";")).arg(destination));
    emit newCopy(orderId,sources,destination);
}

void Listener::moveWithoutDestination(const quint32 &orderId,const QStringList &sources)
{
    ULTRACOPIER_DEBUGCONSOLE(Ultracopier::DebugLevel_Information,QString("moveWithoutDestination(%1,%2)").arg(orderId).arg(sources.join(";")));
    emit newMoveWithoutDestination(orderId,sources);
}

void Listener::move(const quint32 &orderId,const QStringList &sources,const QString &destination)
{
    ULTRACOPIER_DEBUGCONSOLE(Ultracopier::DebugLevel_Information,QString("move(%1,%2,%3)").arg(orderId).arg(sources.join(";")).arg(destination));
    emit newMove(orderId,sources,destination);
}
