#ifndef COLORWHEEL_H
#define COLORWHEEL_H

#include <math.h>

#include <QGridLayout>
#include <QLabel>
#include <QPainter>
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
	}
	
public slots:
	void setValue( double angle, double radius ) {
		x = cos( angle ) * radius * wheelRadius + width() / 2;
		y = -sin( angle ) * radius * wheelRadius + height() / 2;
	}
	
protected:
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
		setValue( a, len );
		emit valueChanged( a, len );
		update();
	}
	
private:
	QPixmap wheel, cursor;
	QRect wheelRect;
	qreal x, y, wheelRadius;
	
signals:
	void valueChanged( double angle, double radius );
};



#define CURSORMID 5
#define CURSORWIDTH 9
#define CURSORHEIGHT 7
#define BARHEIGHT 18
class ColorValueWidget : public QFrame
{
	Q_OBJECT
public:
	ColorValueWidget( QWidget *parent = 0 ) : QFrame( parent ) {
		setMinimumSize( QSize( WHEELSIZE + SELECTORSIZE, BARHEIGHT + CURSORHEIGHT + 2 ) );
		barRect = QRect( CURSORMID, CURSORHEIGHT, width() - CURSORMID * 2 , BARHEIGHT );
		setValue( 0 );
		cursor = QPixmap(":/images/icons/cursor.png");
		gradient = QLinearGradient( QPointF(0, 0), QPointF(1, 0) );
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
	}
	
protected:
	virtual void paintEvent( QPaintEvent* ) {
		QPainter p(this);
		p.setPen( "black" );
		p.setBrush( barBrush );
		barRect.setWidth( width() - CURSORMID * 2 );
		p.drawRect( barRect );
		p.drawPixmap( value * (width() - CURSORMID * 2) + 1, 1, cursor );
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
		double x = p.x() - CURSORMID;
		setValue( x / (width() - CURSORMID * 2) );
		emit valueChanged( value );
		update();
	}
	
private:
	QPixmap cursor;
	QLinearGradient gradient;
	QBrush barBrush;
	QRect barRect;
	qreal value;
	bool pressed;
	
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
	
private:
	ColorWheelWidget *wheel;
	ColorValueWidget *valueBar;
	QGridLayout *grid;
	double hAngle, sRadius, vValue;
};

#endif // COLORWHEEL_H
