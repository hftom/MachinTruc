#include <math.h>

#include <QCursor>

#include "timeline.h"
#include "clipeffectviewitem.h"



ClipEffectViewItem::ClipEffectViewItem( Clip *c, bool video, int id, double scale )
	: AbstractViewItem( 0.75 ),
	clip( c ),
	isVideo( video ),
	index( id )
{
	setData( DATAITEMTYPE, TYPEFILTER );
	setZValue( ZFILTER );
	
	QSharedPointer<Filter> f;
	if ( isVideo )
		f = clip->videoFilters.at( index );
	else
		f = clip->audioFilters.at( index );
	setCuts( f->getPosition() + f->getPositionOffset(), f->getLength(), scale );
	setAcceptHoverEvents(true);

	normalPen.setJoinStyle( Qt::MiterJoin );
	normalPen.setColor( QColor(100,44,0) );

	QLinearGradient grad( QPointF(0, 0), QPointF(0, 1) );
	grad.setCoordinateMode( QGradient::ObjectBoundingMode );
	grad.setColorAt( 0, QColor(255,128,0,220) );
	grad.setColorAt( 1, QColor(128,64,0,220) );
	normalBrush = QBrush( grad );
	
	setPen( normalPen );
	setBrush( normalBrush );
}



void ClipEffectViewItem::mousePressEvent( QGraphicsSceneMouseEvent *event )
{
	firstMove = true;
	if ( event->pos().x() < SNAPWIDTH ) {
		moveResize = 1;
		moveStartLength = length;
		moveStartPosition = position;
	}
	else if ( event->pos().x() > rect().width() - SNAPWIDTH ) {
		moveResize = 2;
		moveStartLength = length;
		moveStartPosition = position;
	}
	else {
		moveResize = 0;
		moveStartPosition = position;
	}		
	moveStartMouse = event->scenePos();
}



void ClipEffectViewItem::mouseMoveEvent( QGraphicsSceneMouseEvent *event )
{
	bool unsnap = event->modifiers() & Qt::ControlModifier;
	if ( firstMove && !unsnap && fabs( event->scenePos().x() - moveStartMouse.x() ) < SNAPWIDTH )
		return;
	firstMove = false;
	Timeline* t = (Timeline*)scene();
	if ( moveResize )
		t->effectCanResize( moveResize, event->scenePos(), moveStartPosition, moveStartLength, moveStartMouse, unsnap );
	else
		t->effectCanMove( event->scenePos(), moveStartPosition, moveStartMouse, unsnap );
}



void ClipEffectViewItem::mouseReleaseEvent( QGraphicsSceneMouseEvent *event )
{
	if ( firstMove )
		return;
	Timeline* t = (Timeline*)scene();
	if ( moveResize )
		t->effectResized( moveResize );
	else
		t->effectMoved( moveStartMouse );
}



void ClipEffectViewItem::hoverMoveEvent( QGraphicsSceneHoverEvent *event )
{
	if ( event->pos().x() < SNAPWIDTH || event->pos().x() > rect().width() - SNAPWIDTH ) {
		setCursor( Qt::SplitHCursor );
	}
	else {
		setCursor( Qt::PointingHandCursor );
	}
}
