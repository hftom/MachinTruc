#ifndef CHECKBOX_H
#define CHECKBOX_H

#include <QCheckBox>
#include <QBoxLayout>

#include "parameterwidget.h"



class CheckBox : public ParameterWidget
{
	Q_OBJECT
public:
	CheckBox( QWidget *parent, Parameter *p );
	QLayout *getLayout() { return box; }
	
	virtual void animValueChanged( double val ) {};
	
private slots:
	void stateChanged( int val );
	
private:
	FSlider *fs;
	QCheckBox *check;
	QBoxLayout *box;
};

#endif // CHECKBOX_H