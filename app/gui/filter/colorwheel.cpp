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
	grid->addWidget( label, 0, 0, 1, 2 );
	
	widgets.append( wheel = new ColorWheelWidget() );
	QColor init = p->value.value<QColor>();
	hAngle = init.redF();
	sRadius = init.greenF();
	vValue = init.blueF();
	wheel->setValue( hAngle * M_PI * 2, sRadius );
	grid->addWidget( wheel, 1, 0, 1, 2 );
	
	grid->addWidget( new QLabel("H:"), 2, 0 );
	grid->addWidget( hueSpin = new QSpinBox(), 2, 1 );
	hueSpin->setRange( 0, 359 );
	hueSpin->setSingleStep( 1 );
	//hueSpin->setSuffix( "°" );
	hueSpin->setValue( hAngle * 359 );
	grid->addWidget( new QLabel("S:"), 3, 0 );
	grid->addWidget( saturationSpin = new QDoubleSpinBox(), 3, 1 );
	saturationSpin->setRange( 0, 1 );
	saturationSpin->setDecimals( 3 );
	saturationSpin->setSingleStep( 0.001 );
	saturationSpin->setValue( sRadius );
	grid->addWidget( new QLabel("V:"), 4, 0 );
	grid->addWidget( valueSpin = new QDoubleSpinBox(), 4, 1 );
	valueSpin->setRange( 0, 1 );
	valueSpin->setDecimals( 3 );
	valueSpin->setSingleStep( 0.001 );
	valueSpin->setValue( vValue );
	
	valueBar = new ColorValueWidget();
	valueBar->setValue( vValue );
	widgets.append( valueBar );
	grid->addWidget( valueBar, 1, 2, 4, 1 );
	
	float r, g, b;
	movit::hsv2rgb(  M_PI * 2 * hAngle, pow( sRadius, 2 ), 1.0f, &r, &g, &b );
	init.setRgbF( r, g, b );
	valueBar->setColor( init );

	grid->setColumnStretch( 2, 1 );

	connect( wheel, SIGNAL(valueChanged(double,double)), this, SLOT(colorChanged(double,double)) );
	connect( valueBar, SIGNAL(valueChanged(double)), this, SLOT(hsvValueChanged(double)) );
	connect( hueSpin, SIGNAL(valueChanged(int)), this, SLOT(hueValueChanged(int)) );
	connect( saturationSpin, SIGNAL(valueChanged(double)), this, SLOT(saturationValueChanged(double)) );
	connect( valueSpin, SIGNAL(valueChanged(double)), this, SLOT(valueValueChanged(double)) );
}



void ColorWheel::hueValueChanged( int v )
{
	hAngle = (double)v / 359.0;
	wheel->setValue( hAngle * M_PI * 2, sRadius );
	newValues();
}



void ColorWheel::saturationValueChanged( double v )
{
	sRadius = v;
	wheel->setValue( hAngle * M_PI * 2, sRadius );
	newValues();
}



void ColorWheel::valueValueChanged( double v )
{
	vValue = v;
	valueBar->setValue( vValue );
	QColor col;
	col.setRgbF( hAngle, sRadius, vValue );
	emit valueChanged( param, col );
}



void ColorWheel::colorChanged( double angle, double radius )
{
	hAngle = angle / (M_PI * 2);
	sRadius = radius;
	hueSpin->blockSignals( true );
	hueSpin->setValue( hAngle * 359 );
	hueSpin->blockSignals( false );
	saturationSpin->blockSignals( true );
	saturationSpin->setValue( sRadius );
	saturationSpin->blockSignals( false );
	newValues();
}



void ColorWheel::hsvValueChanged( double val )
{
	vValue = val;
	valueSpin->blockSignals( true );
	valueSpin->setValue( vValue );
	valueSpin->blockSignals( false );
	QColor col;
	col.setRgbF( hAngle, sRadius, vValue );
	emit valueChanged( param, col );
}



void ColorWheel::newValues()
{
	QColor col;
	float r, g, b;
	movit::hsv2rgb(  M_PI * 2 * hAngle, pow( sRadius, 2 ), 1.0f, &r, &g, &b );
	col.setRgbF( r, g, b );
	valueBar->setColor( col );
	col.setRgbF( hAngle, sRadius, vValue );
	emit valueChanged( param, col );
}



void ColorWheel::animValueChanged( double val )
{
	Q_UNUSED( val );
}
