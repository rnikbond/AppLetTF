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
}
//----------------------------------------------------------------------------------------------------------

ChangesWidget::~ChangesWidget() {

    delete ui;
}
//----------------------------------------------------------------------------------------------------------

void ChangesWidget::reload() {

    m_reloadState = true;

    reloadPrepared();
    reloadExcluded();
    reloadDetected();
}
//----------------------------------------------------------------------------------------------------------

bool ChangesWidget::isReloaded() const {

    return m_reloadState;
}
//----------------------------------------------------------------------------------------------------------

void ChangesWidget::setConfig( const Config& cfg ) {
    m_config = cfg;
}
//----------------------------------------------------------------------------------------------------------

