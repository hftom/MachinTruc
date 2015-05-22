#ifndef FILTERWIDGET_H
#define FILTERWIDGET_H

#include <QWidget>

#include "engine/clip.h"
#include "engine/filter.h"
#include "parameterwidget.h"
#include "shaderedit.h"



class FilterWidget : public QWidget
{
	Q_OBJECT
public:
	FilterWidget( QWidget *parent, Clip *c, QSharedPointer<Filter> f );
	~FilterWidget();
	QSharedPointer<Filter> getFilter() { return filter; }
	void setAnimActive( Parameter *p );
	
private slots:
	void deleteFilter();
	void moveUp();
	void moveDown();
	void valueChanged( Parameter *p, QVariant val );
	void showAnimation( ParameterWidget *pw, Parameter *p ) {
		emit editAnimation( this, pw, p );
	}
	void ovdValueChanged( ParameterWidget *exclude );
	
private:
	Clip *clip;
	QSharedPointer<Filter> filter;
	QList<ParameterWidget*> paramWidgets;
	
signals:
	void filterDeleted( Clip*, QSharedPointer<Filter> );
	void filterSourceDeleted();
	void filterMoveUp( Clip*, QSharedPointer<Filter>);
	void filterMoveDown( Clip*, QSharedPointer<Filter> );
	void editAnimation( FilterWidget*, ParameterWidget*, Parameter* );
	void compileShaderRequest( ThumbRequest );
	void reloadCurrentFilter();
	void updateFrame();
};

#endif // FILTERWIDGET_H
