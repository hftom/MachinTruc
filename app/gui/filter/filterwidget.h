#ifndef FILTERWIDGET_H
#define FILTERWIDGET_H

#include <QWidget>

#include "engine/clip.h"
#include "engine/filter.h"
#include "parameterwidget.h"



class FilterWidget : public QWidget
{
	Q_OBJECT
public:
	FilterWidget( QWidget *parent, Clip *c, Filter *f );
	Filter* getFilter() { return filter; }
	void setAnimActive( Parameter *p );
	
private slots:
	void deleteFilter();
	void valueChanged( Parameter *p, QVariant val );
	void showAnimation( ParameterWidget *pw, Parameter *p ) {
		emit editAnimation( this, pw, p );
	}
	
private:
	Clip *clip;
	Filter *filter;
	QList<ParameterWidget*> paramWidgets;
	
signals:
	void filterDeleted( Clip*, Filter* );
	void filterSourceDeleted();
	void editAnimation( FilterWidget*, ParameterWidget*, Parameter* );
	void updateFrame();
};

#endif // FILTERWIDGET_H
