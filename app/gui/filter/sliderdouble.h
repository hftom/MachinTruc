#ifndef SLIDERDOUBLE_H
#define SLIDERDOUBLE_H

#include <QDoubleSpinBox>
#include <QBoxLayout>
#include <QLabel>

#include "parameterwidget.h"



class SliderDouble : public ParameterWidget
{
	Q_OBJECT
public:
	SliderDouble( QWidget *parent, Parameter *p, bool keyframeable );
	QLayout *getLayout() { return box; }
	
	void animValueChanged( double val );
	
private slots:
	void spinChanged( double val );
	void sliderChanged( int val );
	
private:
	FSlider *fs;
	QDoubleSpinBox *spin;
	QLabel *label;
	QBoxLayout *box;
};

#endif // SLIDERDOUBLE_H
