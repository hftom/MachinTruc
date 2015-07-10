#ifndef STATUSTEXT_H
#define STATUSTEXT_H

#include <QBoxLayout>
#include <QLabel>

#include "parameterwidget.h"



class StatusText : public ParameterWidget
{
	Q_OBJECT
public:
	StatusText( QWidget *parent, Parameter *p );
	QLayout *getLayout() { return box; }
	
	void animValueChanged( double /*val*/ ) {}
	
public slots:
	void statusUpdate( QString s );
	
private:
	QLabel *label, *text;
	QBoxLayout *box;
};

#endif // STATUSTEXT_H