#include <QColorDialog>

#include <movit/util.h>

#include "colorwheel.h"



ColorWheel::ColorWheel( QWidget *parent, Parameter *p, bool keyframeable ) : ParameterWidget( parent, p )
{
	Q_UNUSED( keyframeable );
	
	grid = new QGridLayout();
	grid->setContentsMargins( 0, 0, 0, 0 );
	QLabel *label = new QLabel( p->name );
	widgets.append( label );
	grid->addWidget( label, 0, 0 );
	
	widgets.append( wheel = new ColorWheelWidget() );
	QColor init = p->value.value<QColor>();
	hAngle = init.redF();
	sRadius = init.greenF();
	vValue = init.blueF();
	wheel->setValue( hAngle * M_PI * 2, sRadius );
	grid->addWidget( wheel, 1, 0 );
	
	valueBar = new ColorValueWidget();
	valueBar->setValue( vValue );
	widgets.append( valueBar );
	grid->addWidget( valueBar, 2, 0 );
	
	float r, g, b;
	movit::hsv2rgb(  M_PI * 2 * (1.0 - hAngle), pow( sRadius, 2 ), 1.0f, &r, &g, &b );
	init.setRgbF( r, g, b );
	valueBar->setColor( init );
	
	grid->setColumnStretch( 1, 1 );

	connect( wheel, SIGNAL(valueChanged(double,double)), this, SLOT(colorChanged(double,double)) );
	connect( valueBar, SIGNAL(valueChanged(double)), this, SLOT(hsvValueChanged(double)) );
}



void ColorWheel::colorChanged( double angle, double radius )
{
	hAngle = angle / (M_PI * 2);
	sRadius = radius;
	QColor col;
	float r, g, b;
	movit::hsv2rgb(  M_PI * 2 - angle, pow( radius, 2 ), 1.0f, &r, &g, &b );
	col.setRgbF( r, g, b );
	valueBar->setColor( col );
	col.setRgbF( hAngle, sRadius, vValue );
	emit valueChanged( param, col );
}



void ColorWheel::hsvValueChanged( double val )
{
	vValue = val;
	QColor col;
	col.setRgbF( hAngle, sRadius, vValue );
	emit valueChanged( param, col );	
}



void ColorWheel::animValueChanged( double val )
{
	Q_UNUSED( val );
}
