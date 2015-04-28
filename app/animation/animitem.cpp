#include <math.h>

#include <QPainter>
#include <QDebug>

#include "engine/util.h"
#include "animitem.h"

#define BORDER 5



AnimItem::AnimItem() : QGraphicsRectItem(),
	currentParamWidget( NULL ),
	currentFilterWidget( NULL )
{
	setRect( 0, 0, 10, 10 );
	setPen( QPen( QColor("silver") ) );
	setBrush( QBrush( QColor(255,255,200) ) );
	
	reset();
}



void AnimItem::reset()
{
	if ( currentFilterWidget )
		currentFilterWidget->setAnimActive( NULL );

	if ( currentParamWidget )
		disconnect( currentParamWidget, SIGNAL(keyValueChanged(Parameter*,QVariant)), this, SLOT(keyValueChanged(Parameter*,QVariant)) );
	
	currentParam = NULL;
	currentParamWidget = NULL;
	currentFilterWidget = NULL;
	currentKeyIndex = 0;
}



void AnimItem::removeGraph()
{
	if ( currentParam ) {
		while ( !currentParam->graph.keys.isEmpty() )
			currentParam->graph.keys.takeFirst();
		while ( !keys.isEmpty() )
			delete keys.takeFirst();
		
		sendValue( (currentParam->value.toDouble() - currentParam->min.toDouble()) / (currentParam->max.toDouble() - currentParam->min.toDouble()) );
		reset();
	}
}



void AnimItem::sendValue( double val )
{
	if ( currentParamWidget ) {
		double range = qAbs( -currentParam->min.toDouble() + currentParam->max.toDouble() );
		currentParamWidget->animValueChanged( range * val + currentParam->min.toDouble() );
		emit updateFrame();
	}
}



void AnimItem::keyValueChanged( Parameter *p, QVariant val )
{
	double value;
	
	switch ( p->type ) {
		case Parameter::PDOUBLE: 
		case Parameter::PINPUTDOUBLE: 
			value = val.toInt() / 100.0;
			break;
		case Parameter::PINT:
			value = val.toInt();
			break;
		default:
			return;
	}
	
	QRectF r = rect();
	double w = r.width();
	double h = r.height();
	currentParam->graph.keys[currentKeyIndex].y = (value - currentParam->min.toDouble()) / (currentParam->max.toDouble() - currentParam->min.toDouble());
	double x = currentParam->graph.keys[currentKeyIndex].x * w - HALFKEYSIZE;
	double y = h - currentParam->graph.keys[currentKeyIndex].y * h - HALFKEYSIZE + BORDER;
	keys[currentKeyIndex]->setPos( x, y );
	propagateConstant( currentKeyIndex );
	update();
	emit updateFrame();
}



void AnimItem::setCurrentParam( FilterWidget *f, ParameterWidget *pw, Parameter *p )
{
	reset();
	
	while ( !keys.isEmpty() )
		delete keys.takeFirst();
	
	if ( !p || !pw || !f ) {
		update();
		return;
	}

	currentParam = p;
	currentParamWidget = pw;
	currentFilterWidget = f;
	
	QRectF r = rect();
	double w = r.width();
	double h = r.height();
		
	if ( !currentParam->graph.keys.count() ) {
		double val = (currentParam->value.toDouble() - currentParam->min.toDouble()) / (currentParam->max.toDouble() - currentParam->min.toDouble());
		currentParam->graph.keys.append( AnimationKey( AnimationKey::LINEAR, 0, val ) );
		currentParam->graph.keys.append( AnimationKey( AnimationKey::LINEAR, 1, val ) );
	}
		
	for ( int i = 0; i < currentParam->graph.keys.count(); ++i ) {
		KeyItem *k = new KeyItem( this );
		double x = currentParam->graph.keys[i].x * w - HALFKEYSIZE;
		double y = h - currentParam->graph.keys[i].y * h - HALFKEYSIZE + BORDER;
		k->setPos( x, y );
		keys.append( k );
	}
		
	currentFilterWidget->setAnimActive( currentParam );
	itemSelected( keys.first() );
	connect( currentParamWidget, SIGNAL(keyValueChanged(Parameter*,QVariant)), this, SLOT(keyValueChanged(Parameter*,QVariant)) );

	update();
}



bool AnimItem::filterDeleted( Clip *c, QSharedPointer<Filter> f )
{
	Q_UNUSED( c );
	
	if ( currentFilterWidget && currentFilterWidget->getFilter() == f ) {
		setCurrentParam( NULL, NULL, NULL );
		return true;
	}
	return false;
}



void AnimItem::quitEditor()
{
	reset();
}



void AnimItem::setSize( const QSize &size )
{
	int i;
	
	double w = size.width();
	double h = size.height() - 2 * BORDER;
	setRect( 0, BORDER, w, h );
	if ( currentParam ) {
		for ( i = 0; i < keys.count(); ++i ) {
			double x = currentParam->graph.keys[i].x * w - HALFKEYSIZE;
			double y = h - currentParam->graph.keys[i].y * h - HALFKEYSIZE + BORDER;
			keys[i]->setPos( x, y );
		}
	}
}



void AnimItem::itemSelected( KeyItem *it )
{
	int i;
	
	for ( i = 0; i < keys.count(); ++i ) {
		if ( keys[i] == it ) {
			sendValue( currentParam->graph.keys[i].y );
			currentKeyIndex = i;
		}
		keys[i]->setSelected( false );
	}
	it->setSelected( true );
	update();
}



void AnimItem::itemMove( KeyItem *it, QPointF mouse, QPointF startPos, QPointF startMouse )
{
	int index = keys.indexOf( it );
	
	QPointF newPos = startPos + mouse - startMouse;
	QPointF center = newPos + QPointF( HALFKEYSIZE, HALFKEYSIZE );
	QRectF r = rect();
	r.setBottomRight( r.bottomRight() - QPointF( 1, 1 ) );
	
	center.ry() = qBound( r.y(), center.y(), r.height() + BORDER );
	
	if ( index == 0 )
		center.rx() = 0;
	else if ( index == keys.count() - 1 )
		center.rx() = r.width();
	else 
		center.rx() = qBound( keys[index-1]->scenePos().x() + HALFKEYSIZE + 1.5, center.x(), keys[index+1]->scenePos().x() + HALFKEYSIZE - 1.5 );
	
	currentParam->graph.keys[index].x = center.x() / r.width();
	currentParam->graph.keys[index].y = 1.0 - (center.y() - BORDER) / r.height();
	it->setPos( center - QPointF( HALFKEYSIZE, HALFKEYSIZE ) );
	
	propagateConstant( index );
	sendValue( currentParam->graph.keys[index].y );
	update();
}



void AnimItem::mouseDoubleClickEvent( QGraphicsSceneMouseEvent *event )
{
	int i;
	
	if ( !currentParam )
		return;
	
	QRectF r = rect();
	r.setBottomRight( r.bottomRight() - QPointF( 1, 1 ) );
	double x = event->scenePos().x() / r.width();
	double y = 1.0 - event->scenePos().y() / r.height();

	for ( i = 0; i < currentParam->graph.keys.count(); ++i ) {
		if ( x < currentParam->graph.keys[i].x ) {
			currentParam->graph.keys.insert( i, AnimationKey( AnimationKey::LINEAR, x, y ) );
			keys.insert( i, new KeyItem( this ) );
			keys[i]->setPos( event->scenePos() - QPointF( HALFKEYSIZE, HALFKEYSIZE ) );
			itemMove( keys[i], QPointF( 0, 0 ), keys[i]->pos(), QPointF( 0, 0 ) );
			itemSelected( keys[i] );
			break;
		}
	}
	
	emit updateFrame();
}



void AnimItem::keyDoubleClicked( KeyItem *it )
{
	int index = keys.indexOf( it );
	
	if ( currentParam->graph.keys[index].keyType == AnimationKey::CONSTANT )
		currentParam->graph.keys[index].keyType = AnimationKey::LINEAR;
	else if ( currentParam->graph.keys[index].keyType == AnimationKey::LINEAR )
		currentParam->graph.keys[index].keyType = AnimationKey::CURVE;
	else {
		currentParam->graph.keys[index].keyType = AnimationKey::CONSTANT;
		propagateConstant( index );
	}

	update();
	emit updateFrame();
}



void AnimItem::propagateConstant( int index )
{
	int i = index;
	while ( i != keys.count() - 1 && currentParam->graph.keys[i].keyType == AnimationKey::CONSTANT ) {
		currentParam->graph.keys[i+1].y = currentParam->graph.keys[i].y;
		keys[i+1]->setY( keys[i]->y() );
		++i;
	}
	i = index;
	while ( i > 0 && currentParam->graph.keys[i-1].keyType == AnimationKey::CONSTANT ) {
		currentParam->graph.keys[i-1].y = currentParam->graph.keys[i].y;
		keys[i-1]->setY( keys[i]->y() );
		--i;
	}
}



void AnimItem::paint( QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget )
{
	QGraphicsRectItem::paint( painter, option, widget );
	
	if ( !currentParam )
		return;

	int i, j, w = rect().width(), h = rect().height();
	QPen pen;
	pen.setWidth( 1 );

	for ( i = 0; i < currentParam->graph.keys.count() - 1; ++i ) {
		int x1 = currentParam->graph.keys[i].x * w;
		int x2 = currentParam->graph.keys[i+1].x * w;
		double y1 = currentParam->graph.keys[i].y;
		double y2 = currentParam->graph.keys[i+1].y;
		
		double ipol;
		if ( currentParam->graph.keys[i].keyType == AnimationKey::LINEAR ) {
			pen.setColor( QColor("green") );
			painter->setPen( pen );
			ipol = linearInterpolate( y1, y2, 0 );
		}
		else if ( currentParam->graph.keys[i].keyType == AnimationKey::CURVE ) {
			pen.setColor( QColor("red") );
			painter->setPen( pen );
			ipol = cosineInterpolate( y1, y2, 0 );
		}
		else {
			pen.setColor( QColor("blue") );
			painter->setPen( pen );
			ipol = y1;
		}
		int lastX = x1;
		int lastY = h - (ipol * h);
		painter->drawPoint( lastX, lastY );
		for ( j = x1 + 1; j < x2 + 1; ++j ) {
			if ( currentParam->graph.keys[i].keyType == AnimationKey::LINEAR )
				ipol = linearInterpolate( y1, y2, (double)(j - x1) / (double)( x2 - x1 ) );
			else if ( currentParam->graph.keys[i].keyType == AnimationKey::CURVE )
				ipol = cosineInterpolate( y1, y2, (double)(j - x1) / (double)( x2 - x1 ) );
			else
				ipol = y1;
			int y = h - (ipol * h);
			painter->drawLine( lastX, lastY + BORDER, j, y + BORDER );
			lastX = j;
			lastY = y;
		}
	}
}
