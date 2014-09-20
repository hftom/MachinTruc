#include <math.h>

#include <QCursor>

#include "transitionviewitem.h"



TransitionViewItem::TransitionViewItem( QGraphicsItem *parent, double pos, double len, double scale )
{
	setData( DATAITEMTYPE, TYPETRANSITION );	
	setParentItem( parent );
	setZValue( ZTRANSITION );

	//setAcceptHoverEvents(true);
	
	setCuts( pos, len, scale );
	
	normalPen.setJoinStyle( Qt::MiterJoin );
	normalPen.setColor( "lime" );

	QLinearGradient grad( QPointF(0, 0), QPointF(0, 1) );
	grad.setCoordinateMode( QGradient::ObjectBoundingMode );
	grad.setColorAt( 0, QColor(200,255,0,100) );
	grad.setColorAt( 1, QColor(128,178,0,100) );
	normalBrush = QBrush( grad );
	
	setPen( normalPen );
	setBrush( normalBrush );
}



void TransitionViewItem::paint( QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget )
{
	QGraphicsRectItem::paint( painter, option, widget );
		
	/*QRectF inside = rect();
	inside.moveLeft( inside.x() + 1 );
	inside.moveTop( inside.y() + 1 );
	inside.setWidth( inside.width() - 2 );
	inside.setHeight( inside.height() - 2 );
	QRectF r = painter->boundingRect( inside, Qt::AlignHCenter, QFileInfo(filename).fileName() );
	if ( r.width() > inside.width() ) {
		r.setX( inside.x() );
		r.setWidth( inside.width() );
	}
	
	painter->setPen( QColor(0,0,0,0) );
	if ( selected )
		painter->setBrush( titleSelectionBrush );
	else
		painter->setBrush( titleNormalBrush );
	
	if ( r.width() > 0 ) {
		painter->drawRect( r );
		painter->setPen( QColor(255,255,255) );
		painter->setFont( QFont( "Sans", 8 ) );
		painter->drawText( r, Qt::AlignHCenter, QFileInfo(filename).fileName() );
	}*/
}



/*void TransitionViewItem::setSelected( bool b )
{
	selected = b;
	if ( selected ) {
		setBrush( selectionBrush );
		setPen( selectionPen );
	}
	else {
		setPen( normalPen );
		setBrush( normalBrush );
	}
	update();
}



void TransitionViewItem::mousePressEvent( QGraphicsSceneMouseEvent *event )
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
	Timeline* t = (Timeline*)scene();
	t->itemSelected( this );
}



void TransitionViewItem::mouseMoveEvent( QGraphicsSceneMouseEvent *event )
{
	bool unsnap = event->modifiers() & Qt::ControlModifier;
	if ( firstMove && !unsnap && fabs( event->scenePos().x() - moveStartMouse.x() ) < SNAPWIDTH )
		return;
	firstMove = false;
	Timeline* t = (Timeline*)scene();
	if ( moveResize )
		t->clipItemCanResize( this, moveResize, event->scenePos(), moveStartPosition, moveStartLength, moveStartMouse, unsnap );
	else
		t->clipItemCanMove( this, event->scenePos(), moveStartPosition, moveStartMouse, unsnap );
}



void TransitionViewItem::mouseReleaseEvent( QGraphicsSceneMouseEvent *event )
{
	Q_UNUSED( event );
	
	if ( firstMove )
		return;
	Timeline* t = (Timeline*)scene();
	if ( moveResize )
		t->clipItemResized( this, moveResize );
	else
		t->clipItemMoved( this, moveStartMouse );
}



void TransitionViewItem::hoverMoveEvent( QGraphicsSceneHoverEvent *event )
{
	if ( event->pos().x() < SNAPWIDTH || event->pos().x() > rect().width() - SNAPWIDTH ) {
		setCursor( QCursor(Qt::SplitHCursor) );
	}
	else {
		setCursor( QCursor(Qt::PointingHandCursor) );
	}
}



void TransitionViewItem::dragEnterEvent( QGraphicsSceneDragDropEvent *event )
{
	const QMimeData *mimeData = event->mimeData();
	if ( mimeData->formats().contains( MIMETYPEEFFECT ) ) {
		QString t = mimeData->data( MIMETYPEEFFECT ).data();
		event->accept();
	}
	else
		event->ignore();
}



void TransitionViewItem::dropEvent( QGraphicsSceneDragDropEvent *event )
{
	const QMimeData *mimeData = event->mimeData();
	if ( mimeData->formats().contains( MIMETYPEEFFECT ) ) {
		QString t = mimeData->data( MIMETYPEEFFECT ).data();
		Timeline* tm = (Timeline*)scene();
		tm->addFilter( this, t );
		event->accept();
	}
}*/
