//----------------------------------------
#include <QDebug>
#include <QTextCodec>
#include <QApplication>
//----------------------------------------
#include "TFRequest.h"
//----------------------------------------

TFRequest::TFRequest( QObject* parent ) : QObject(parent) {

#ifdef WIN32
    m_codec = QTextCodec::codecForName("cp1251");
#else
    m_codec = QTextCodec::codecForName("utf8");
#endif

    m_isTouchCursor = true;
    m_tf = new QProcess( this );

    clear();

    connect( m_tf, QOverload<int,QProcess::ExitStatus>::of(&QProcess::finished)              , this, &TFRequest::reactOnFinished      );
    connect( m_tf,                                         &QProcess::stateChanged           , this, &TFRequest::reactOnStateChanged  );
    connect( m_tf,                                         &QProcess::errorOccurred          , this, &TFRequest::reactOnErrorOccurred );
}
//----------------------------------------------------------------------------------------------------------

void TFRequest::workspaces() {

     m_cmd = CommandWorkspaces;

    QStringList args = {
        "workspaces",
         QString("-login:%1,%2").arg(m_config.m_azure.login, m_config.m_azure.password),
         QString("-collection:%1").arg(m_config.m_azure.url),
    };

    m_tf->start( m_config.m_azure.tfPath, args );
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Обработка изменения состояния процесса программы TF
 * \param state Состояние
 */
void TFRequest::reactOnStateChanged( QProcess::ProcessState state ) {

    switch( state ) {
        case QProcess::Starting: {
            clear();
            if( m_isTouchCursor ) {
                QApplication::setOverrideCursor( Qt::WaitCursor );
            }
            break;
        }
        default: break;
    }
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Обработка данных после завершения работы программы TF
 * \param exitCode   Код завершения работы программы
 * \param exitStatus Статус завершения работы программы
 */
void TFRequest::reactOnFinished( int exitCode, QProcess::ExitStatus exitStatus ) {

    Q_UNUSED( exitStatus );

    m_errCode = exitCode;

    switch( m_errCode ) {
        case 0: {
            m_response = m_codec->toUnicode(m_tf->readAllStandardOutput()).split("\n", Qt::SkipEmptyParts);
            break;
        }
        default: {
            m_errText = m_codec->toUnicode(m_tf->readAllStandardError());
            break;
        }
    }

    qDebug() << "m_err_code:" << m_errCode;
    qDebug() << "m_err_text:" << m_errText;
    qDebug() << "m_response:" << m_response;

    emit executed();

    if( m_isTouchCursor ) {
        QApplication::restoreOverrideCursor();
    }
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Обработка ошибки запуска процесса
 * \param error Информация об ошибке
 *
 * Возникает, если не удается запустить программу TF.
 * При вызове этого слота, слот \a reactOnFinished() не вызывается.
 */
void TFRequest::reactOnErrorOccurred( QProcess::ProcessError error ) {

    qDebug() << QString("reactOnErrorOccurred(%1)").arg(error);

    m_errCode = error;

    switch( m_errCode ) {
        case QProcess::FailedToStart: m_errText = tr("Не удалось запустить программу TF"); break;
        default                     : m_errText = m_tf->errorString();                     break;
    }

    qDebug() << "m_err_code:" << m_errCode;
    qDebug() << "m_err_text:" << m_errText;
    qDebug() << "m_response:" << m_response;

    emit executed();

    if( m_isTouchCursor ) {
        QApplication::restoreOverrideCursor();
    }
}
//----------------------------------------------------------------------------------------------------------

void TFRequest::setConfig( const Config& cfg ) {

    m_config = cfg;
}
//----------------------------------------------------------------------------------------------------------

void TFRequest::clear() {

    m_isErr   = false;
    m_errCode = 0;
    m_errText .clear();
    m_response.clear();
}
//----------------------------------------------------------------------------------------------------------

