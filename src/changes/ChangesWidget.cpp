//----------------------------------------
#include <QDir>
#include <QMenu>
#include <QAction>
#include <QSettings>
#include <QTextStream>
#include <QAbstractTextDocumentLayout>
//----------------------------------------
#include "methods.h"
#include "TFRequest.h"
//----------------------------------------
#include "ChangesWidget.h"
#include "ui_ChangesWidget.h"
//----------------------------------------
#define CHANGES_FILE ".changes.dat"
//----------------------------------------

ChangesWidget::ChangesWidget( QWidget* parent ) : QWidget(parent), ui(new Ui::ChangesWidget)
{
    ui->setupUi(this);

    setupPrepared();
    setupExcluded();
    setupDetected();

    m_reloadAction = new QAction( QIcon(":/reload.png"), tr("Обновить"), this );

    m_reloadState = false;

#ifdef WIN32
    m_statusesTfsMap = {
        { tr("добавление"), CreateStatus },
        { tr("изменить"  ), ChangeStatus },
        { tr("удалить"   ), DeleteStatus },
    };
#else
    m_statusesTfsMap = {
        { tr("add"   ), CreateStatus },
        { tr("edit"  ), ChangeStatus },
        { tr("delete"), DeleteStatus },
    };
#endif

    connect( m_reloadAction, &QAction::triggered, this, &ChangesWidget::reload );
}
//----------------------------------------------------------------------------------------------------------

ChangesWidget::~ChangesWidget() {

    delete ui;
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Сохранение данных
 */
void ChangesWidget::saveData() {

    QString path = QString("%1/%2").arg(workDirPath(), CHANGES_FILE);
    QFile file(path);
    if( !file.open(QIODevice::WriteOnly) ) {
        return;
    }

    QTextStream out( &file );
    out << m_excluded.join(";");
    file.close();
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Восстановление данных
 */
void ChangesWidget::restoreData() {

    QString path = QString("%1/%2").arg(workDirPath(), CHANGES_FILE);
    QFile file(path);
    if( !file.open(QIODevice::ReadOnly) ) {
        return;
    }

    QString data = file.readAll();
    file.close();

    m_excluded = data.split(";");
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Получение списка действий
 * \return
 */
QList<QAction*> ChangesWidget::actions() const {

    return {
             m_preparedDiffAction  ,
             m_preparedCancelAction,
             m_reloadAction        ,
            };
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Перезагрузка изменений
 */
void ChangesWidget::reload() {

    m_reloadState = true;

    reloadPrepared();
    reloadExcluded();
    reloadDetected();
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Получение признака, были ли перезагружены изменения
 * \return TRUE, если изменения были перезагружены. Иначе FALSE.
 */
bool ChangesWidget::isReloaded() const {

    return m_reloadState;
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Изменение конфигурации
 * \param cfg Новая конфигурация
 */
void ChangesWidget::setConfig( const Config& cfg ) {
    m_config = cfg;
}
//----------------------------------------------------------------------------------------------------------

