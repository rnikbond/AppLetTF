//----------------------------------------
#include <QDir>
#include <QMenu>
#include <QAction>
#include <QSettings>
#include <QAbstractTextDocumentLayout>
//----------------------------------------
#include "TFRequest.h"
//----------------------------------------
#include "ChangesWidget.h"
#include "ui_ChangesWidget.h"
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

