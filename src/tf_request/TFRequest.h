//----------------------------------------
#ifndef TFREQUEST_H
#define TFREQUEST_H
//----------------------------------------
#include <QObject>
#include <QProcess>
//----------------------------------------
#include "Config.h"
//----------------------------------------

class TFRequest : public QObject {

    Q_OBJECT

    QProcess  * m_tf;
    QTextCodec* m_codec;
    Config      m_config;

public:

    enum Commands {
        CommandNone      , ///< Отсутствие команды
        CommandWorkspaces, ///< Команда "workspaces"
    };

public:

    Commands    m_cmd;
    QStringList m_response;

    bool    m_isErr;
    int     m_errCode;
    QString m_errText;

    bool m_isTouchCursor;

public:

    TFRequest( QObject* parent = nullptr );

    void setConfig( const Config& cfg );
    void clear();

public: // Read Requests

    void workspaces();

private:

    void reactOnStateChanged( QProcess::ProcessState state );
    void reactOnFinished( int exitCode, QProcess::ExitStatus exitStatus );
    void reactOnErrorOccurred( QProcess::ProcessError error );

signals:

    void executed();
};
//----------------------------------------------------------------------------------------------------------

#endif // TFREQUEST_H
