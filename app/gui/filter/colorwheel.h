#ifndef COLORWHEEL_H
#define COLORWHEEL_H

#include <math.h>

#include <QGridLayout>
#include <QLabel>
#include <QPainter>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QDebug>

#include "parameterwidget.h"


#define WHEELSIZE 112
#define SELECTORRADIUS 5
#define SELECTORSIZE SELECTORRADIUS * 2


class ColorWheelWidget : public QFrame
{
	Q_OBJECT
public:
	ColorWheelWidget( QWidget *parent = 0 ) : QFrame( parent ) {
		wheelRect = QRect( SELECTORRADIUS, SELECTORRADIUS, WHEELSIZE, WHEELSIZE );
		wheelRadius = WHEELSIZE / 2;
		setFixedSize( QSize( WHEELSIZE + SELECTORSIZE, WHEELSIZE + SELECTORSIZE ) );
		setValue( 0, 0 );
		wheel = QPixmap(":/images/icons/color-wheel.png");
		// A linear function makes the wheel quite useless
		// for setting the Saturation since interesting values
		// are too close to the center.
		// A pow function is more suited.
		powerOf = 2.5;
	}

public slots:
	void setValue( double angle, double radius ) {
		radius = pow( radius, 1.0 / powerOf );
		x = cos( angle ) * radius * wheelRadius + width() / 2;
		y = -sin( angle ) * radius * wheelRadius + height() / 2;
		update();
	}

protected:
	double powerOf;

	virtual void paintEvent( QPaintEvent* ) {
		QPainter p(this);
		p.drawPixmap( wheelRect, wheel );
		p.setPen( "black" );
		p.drawEllipse( x - SELECTORRADIUS, y - SELECTORRADIUS, SELECTORSIZE, SELECTORSIZE );
		p.setPen( "white" );
		p.drawEllipse( x - SELECTORRADIUS + 1, y - SELECTORRADIUS + 1, SELECTORSIZE - 2, SELECTORSIZE - 2 );
	}

	virtual void mouseDoubleClickEvent( QMouseEvent* ) {
		move( QPoint( width() / 2, width() / 2 ) );
	}

	virtual void mousePressEvent( QMouseEvent *event ) {
		move( event->pos() );
	}

	virtual void mouseMoveEvent( QMouseEvent *event ) {
		move( event->pos() );
	}

	void move( QPoint p ) {
		double x1 = p.x() - width() / 2;
		double y1 = p.y() - height() / 2;
		double len = qMin( 1.0, sqrt( x1 * x1 + y1 * y1 ) / wheelRadius );
		double a = atan2( -y1 , x1 );
		if ( a < 0 )
			a += M_PI * 2;
		x = cos( a ) * len * wheelRadius + width() / 2;
		y = -sin( a ) * len * wheelRadius + height() / 2;
		update();
		emit valueChanged( a, pow( len, powerOf ) );
	}

private:
	QPixmap wheel, cursor;
	QRect wheelRect;
	qreal x, y, wheelRadius;

signals:
	void valueChanged( double angle, double radius );
};



#define CURSORMID 5
#define CURSORWIDTH 7
#define CURSORHEIGHT 9
#define BARWIDTH 15

class ColorValueWidget : public QFrame
{
	Q_OBJECT
public:
	ColorValueWidget( QWidget *parent = 0 ) : QFrame( parent ) {
		setMinimumSize( QSize( BARWIDTH + 2, WHEELSIZE + SELECTORSIZE ) );
		barRect = QRect( 0, CURSORMID, BARWIDTH, height() - CURSORMID * 2 );
		setValue( 0 );
		cursor = QPixmap(":/images/icons/cursor.png");
		gradient = QLinearGradient( QPointF(0, 1), QPointF(0, 0) );
		gradient.setCoordinateMode( QGradient::ObjectBoundingMode );
		gradient.setColorAt( 0, QColor("black") );
		gradient.setColorAt( 1, QColor("white") );
		barBrush = QBrush( gradient );
	}

public slots:
	void setColor( const QColor &col ) {
		gradient.setColorAt( 1, col );
		barBrush = QBrush( gradient );
		update();
	}

	void setValue( double val ) {
		value = qMax( qMin( 1.0, val ), 0.0 );
		update();
	}

protected:
	virtual void paintEvent( QPaintEvent* ) {
		QPainter p(this);
		p.setPen( "black" );
		p.setBrush( barBrush );
		barRect.setHeight( height() - CURSORMID * 2 );
		p.drawRect( barRect );
		p.drawPixmap( 0, height() - CURSORHEIGHT - (value * (height() - CURSORMID * 2)), cursor );
	}

	/*virtual void mouseDoubleClickEvent( QMouseEvent* ) {
		move( QPoint( width() / 2, width() / 2 ) );
	}*/

	virtual void mousePressEvent( QMouseEvent *event ) {
		move( event->pos() );
	}

	virtual void mouseMoveEvent( QMouseEvent *event ) {
		move( event->pos() );
	}

	void move( QPoint p ) {
		double y = p.y() - CURSORMID;
		y /= height() - CURSORMID * 2;
		setValue( 1.0 - y );
		emit valueChanged( value );
	}

private:
	QPixmap cursor;
	QLinearGradient gradient;
	QBrush barBrush;
	QRect barRect;
	qreal value;

signals:
	void valueChanged( double val );
};



class ColorWheel : public ParameterWidget
{
	Q_OBJECT
public:
	ColorWheel( QWidget *parent, Parameter *p, bool keyframeable );
	QLayout *getLayout() { return grid; }

	void animValueChanged( double val );

private slots:
	void colorChanged( double angle, double radius );
	void hsvValueChanged( double val );
	void hueValueChanged( int v );
	void saturationValueChanged( double v );
	void valueValueChanged( double v );

private:
	void newValues();

	ColorWheelWidget *wheel;
	ColorValueWidget *valueBar;
	QSpinBox *hueSpin;
	QDoubleSpinBox *saturationSpin, *valueSpin;
	QGridLayout *grid;
	double hAngle, sRadius, vValue;
};

#endif // COLORWHEEL_H
