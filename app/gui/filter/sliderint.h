#ifndef SLIDERINT_H
#define SLIDERINT_H

#include <QSpinBox>
#include <QBoxLayout>
#include <QLabel>

#include "parameterwidget.h"



class SliderInt : public ParameterWidget
{
	Q_OBJECT
public:
	SliderInt( QWidget *parent, Parameter *p, bool keyframeable );
	QLayout *getLayout() { return box; }
	
	void animValueChanged( double val );
	
private slots:
	void spinChanged( int val );
	void sliderChanged( int val );
	
private:
	FSlider *fs;
	QSpinBox *spin;
	QLabel *label;
	QBoxLayout *box;
};

#endif // SLIDERINT_H