#ifndef IMPUTDOUBLE_H
#define IMPUTDOUBLE_H

#include <QDoubleSpinBox>
#include <QBoxLayout>
#include <QLabel>

#include "parameterwidget.h"



class InputDouble : public ParameterWidget
{
	Q_OBJECT
public:
	InputDouble( QWidget *parent, Parameter *p, bool keyframeable );
	QLayout *getLayout() { return box; }
	
	virtual void animValueChanged( double val );
	virtual void ovdValueChanged();
	
private slots:
	void spinChanged( double val );
	
private:
	QDoubleSpinBox *spin;
	QLabel *label;
	QBoxLayout *box;
};

#endif // IMPUTDOUBLE_H
