#include <QDebug>
#include <QCursor>

#include "engine/profile.h"
#include "timeline.h"
#include "typeitem.h"
#include "rulerviewitem.h"



RulerViewItem::RulerViewItem() : frameDuration(40000), lastY(0), docked(true)
{
	setData( DATAITEMTYPE, TYPERULER );
	setRect( 0, 0, RULERWIDTH, RULERHEIGHT );
	
	background = QPixmap( RULERWIDTH, RULERHEIGHT );
	background.fill( QColor(0,0,0,0) );
	QPainter p;
	p.begin( &background );
	p.setRenderHints( QPainter::Antialiasing );
	p.setPen( QColor(0,0,0,0) );
	QLinearGradient grad( QPointF(0, 0), QPointF(1, 0) );
	grad.setCoordinateMode( QGradient::ObjectBoundingMode );
	grad.setColorAt( 0, QColor(255,255,150,10) );
	grad.setColorAt( 0.05, QColor(255,255,150,140) );
	grad.setColorAt( 0.5, QColor(255,255,0,180) );
	grad.setColorAt( 0.95, QColor(255,255,150,140) );
	grad.setColorAt( 1, QColor(255,255,150,10) );
	p.setBrush( QBrush( grad ) );
	p.drawRoundedRect( QRectF( 0, 0, RULERWIDTH, RULERHEIGHT ), RULERHEIGHT / 2, RULERHEIGHT / 2 );
	p.end();
	
	anim = new QPropertyAnimation( this, "posy" );
	anim->setDuration( 250 );
	anim->setEasingCurve( QEasingCurve::InOutSine );
	
	int fontSize = 10;
	font = QFont( "Arial", fontSize );
	QFontMetrics fm( font );
	while (fontSize > 0 && fm.boundingRect( "99:99:99" ).height() > 10) {
		font = QFont( "Arial", --fontSize );
		fm =  QFontMetrics( font );
	}
	currentTextLen = textShortLen = fm.boundingRect( "99:99:99" ).width();
	textLongLen = fm.boundingRect( "99:99:99.99" ).width();
}



void RulerViewItem::setPosition( qreal posx, qreal posy )
{
	setX( posx );
	if ( posy == lastY )
		return;
	anim->stop();
	anim->setKeyValueAt( 0, y() );
	anim->setKeyValueAt( 1, posy );
	anim->start();
	lastY = posy;
}



void RulerViewItem::setTimeScale( double pps )
{
	pixelsPerUnit = pps;
	currentTextLen = textShortLen;
	double target = 18.0;
	do {
		ticklen = target / pps;
		tickdistance = pps * ticklen;
		target += 1;
	} while ( tickdistance < 18 && ticklen > 0 );
	if ( ticklen < 1 ) {
		currentTextLen = textLongLen;
		pixelsPerUnit = frameDuration * pps / MICROSECOND;
		target = 18.0;
		do {
			ticklen = target / pixelsPerUnit;
			tickdistance = pixelsPerUnit * ticklen;
			target += 1;
		} while ( tickdistance < 18 && ticklen > 0 );
		if ( ticklen < 1 ) {
			ticklen = 1;
			tickdistance = pixelsPerUnit * ticklen;
		}
	}
}



#define FADELENGTH 50
#define NTICKS 5
void RulerViewItem::paint( QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget )
{
	Q_UNUSED( option );
	Q_UNUSED( widget );

	QRectF r = rect();
	painter->setRenderHints( QPainter::Antialiasing | QPainter::TextAntialiasing );
	painter->drawPixmap( 0, 0, background );
	
	painter->setFont( font );
	QColor tickColor( 0, 0, 0, 0 );
	double rw = r.width(), rh = r.height();
	unsigned n = x() / tickdistance;
	double start = (tickdistance * n) - x() + tickdistance;
	double tlen = (currentTextLen == textShortLen) ? ticklen : ticklen * frameDuration / MICROSECOND;
	
	// search for the previous time position since we want
	// to display the tail of this text.
	double prev = start - (tickdistance * NTICKS);
	double firstText = prev;
	double ip;
	while ( prev < rw ) {
		double fract = modf( (((x() + prev) / pixelsPerUnit) / (ticklen * NTICKS) + 1e-6), &ip );
		if (  fract < 1e-3 ) {
			firstText = prev;
			break;
		}
		prev += tickdistance;
	}
	// draw it
	if ( firstText + currentTextLen > 0 ) {
		QTime time = QTime(0,0).addMSecs( tlen * NTICKS * ip * 1000 );
		painter->setPen( QPen( QBrush( getTextGradient( firstText, currentTextLen, false ) ), 0 ) );
		if ( currentTextLen == textShortLen )
			painter->drawText( QRectF( firstText, 0.0,  rw - firstText, rh ), time.toString("hh:mm:ss"), Qt::AlignTop | Qt::AlignLeft );
		else
			painter->drawText( QRectF( firstText, 0.0,  rw - firstText, rh ), QString("%1.%2").arg( time.toString("hh:mm:ss") )
				.arg( (int)( time.msec() / (frameDuration / MILLISECOND) + 0.5), 2, 10, QChar('0') ), Qt::AlignTop | Qt::AlignLeft );
	}
	
	while ( start < rw ) {
		if ( start < FADELENGTH )
			tickColor.setAlpha( start * 255 / FADELENGTH );
		else if ( start > RULERWIDTH - FADELENGTH )
			tickColor.setAlpha( (RULERWIDTH - start) * 255 / FADELENGTH );
		else
			tickColor.setAlpha( 255 );
		painter->setPen( tickColor );
		double fract = modf( (((x() + start) / pixelsPerUnit) / (ticklen * NTICKS) + 1e-6), &ip );
		if (  fract < 1e-3 ) {
			painter->drawLine( start, RULERHEIGHT / 2, start, rh - 1 );
			QTime time = QTime(0,0).addMSecs( tlen * NTICKS * ip * MILLISECOND );
			if ( start < FADELENGTH )
				painter->setPen( QPen( QBrush( getTextGradient( start, currentTextLen, false ) ), 0 ) );
			else if ( start > rw - FADELENGTH - currentTextLen )
				painter->setPen( QPen( QBrush( getTextGradient( rw - start - currentTextLen, currentTextLen, true ) ), 0 ) );
			if ( currentTextLen == textShortLen )
				painter->drawText( QRectF( start, 0.0,  rw - start, rh ), time.toString("hh:mm:ss"), Qt::AlignTop | Qt::AlignLeft );
			else
				painter->drawText( QRectF( start, 0.0,  rw - start, rh ), QString("%1.%2").arg( time.toString("hh:mm:ss") )
					.arg( (int)( time.msec() / (frameDuration / MILLISECOND) + 0.5), 2, 10, QChar('0') ), Qt::AlignTop | Qt::AlignLeft );
		}
		else {
			painter->drawLine( start, RULERHEIGHT * 2 / 3, start, rh - 1 );
		}
		
		start += tickdistance;
	}
	
	painter->setPen( "blue" );
	if ( currentTextLen == textShortLen )
		painter->drawText( 60, rh - 2, QString("%1s").arg( ticklen ) );
	else
		painter->drawText( 60, rh - 2, QString("%1i").arg( ticklen ) );
}



QLinearGradient RulerViewItem::getTextGradient( double start, double textLen, bool revert )
{
	double p1 = 0.0, p2 = 1.0, v1 = 0.0, v2 = 1.0;
	
	if ( start < 0 )
		p1 = -start / textLen;
	else
		v1 = start / FADELENGTH;
	
	if ( (start + textLen) > FADELENGTH )
		p2 = (FADELENGTH - start) / textLen;
	else
		v2 = (start + textLen) / FADELENGTH;
	
	QLinearGradient grad;
	grad.setCoordinateMode( QGradient::ObjectBoundingMode );
	if ( revert ) {
		grad.setFinalStop( QPointF(0, 0) );
		grad.setStart( QPointF(1, 0) );
	}
	else {
		grad.setStart( QPointF(0, 0) );
		grad.setFinalStop( QPointF(1, 0) );
	}
	grad.setColorAt( qMin(1.0, qMax(0.0, p1)), QColor(0, 0, 0, qMin(255, qMax(0, (int)(v1 * 255))) ) );
	grad.setColorAt( qMin(1.0, qMax(0.0, p2)), QColor(0, 0, 0, qMin(255, qMax(0, (int)(v2 * 255))) ) );
	
	return grad;
}
