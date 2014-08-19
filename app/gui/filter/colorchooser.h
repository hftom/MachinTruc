#ifndef COLORCHOOSER_H
#define COLORCHOOSER_H

#include <QPushButton>
#include <QBoxLayout>
#include <QLabel>

#include "parameterwidget.h"



class ColorChooser : public ParameterWidget
{
	Q_OBJECT
public:
	ColorChooser( QWidget *parent, Parameter *p, bool keyframeable );
	QLayout *getLayout() { return box; }
	
	void animValueChanged( double val );
	
private slots:
	void showDialog();
	void colorChanged( const QColor &col );
	
private:
	QPushButton *btn;
	QLabel *label;
	QBoxLayout *box;
};

#endif // COLORCHOOSER_H
