#ifndef FXPAGE_H
#define FXPAGE_H

#include <QWidget>

#include "ui_fxpage.h"
#include "engine/clip.h"
#include "gui/filter/filterwidget.h"



class FxPage : public QWidget, private Ui::StackFx
{
	Q_OBJECT
public:
	FxPage();
	
public slots:
	void clipSelected( Clip *clip );
	
private slots:
	void deletedFilter( Clip *c, QSharedPointer<Filter> f );
	void filterMoveUp( Clip *c, QSharedPointer<Filter> f );
	void filterMoveDown( Clip *c, QSharedPointer<Filter> f );

private:
	QWidget *currentEffectsWidget;
	QGridLayout *effectsWidgetLayout;
	QList<FilterWidget*> filterWidgets;
	
signals:
	void filterDeleted( Clip*, QSharedPointer<Filter> );
	void editAnimation( FilterWidget*, ParameterWidget*, Parameter* );
	void updateFrame();
};

#endif // FXPAGE_H