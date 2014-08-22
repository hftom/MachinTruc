#include <math.h>

#include <QFileInfo>
#include <QMimeData>

#include "gui/mimetypes.h"
#include "timeline.h"
#include "clipviewitem.h"



ClipViewItem::ClipViewItem( Clip *c, double scale ) : AbstractViewItem( VIDEOCUT )
{
	//setFlag( QGraphicsItem::ItemIsMovable, true );
	//setFlag(QGraphicsItem::ItemIsSelectable, true);
	//setFlag(QGraphicsItem::ItemIsFocusable, true);
	setAcceptHoverEvents(true);
	setAcceptDrops( true );
	 
	setClip( c );
	filename = clip->sourcePath();
	
	setCuts( clip->position(), clip->length() );
	setScale( scale );

	normalPen.setJoinStyle( Qt::MiterJoin );
	normalPen.setColor( QColor(0, 0, 255) );
	
	selectionPen.setJoinStyle( Qt::MiterJoin );
	selectionPen.setColor( QColor(255, 0, 0) );

	QLinearGradient grad( QPointF(0, 0), QPointF(0, 1) );
	grad.setCoordinateMode( QGradient::ObjectBoundingMode );
	grad.setColorAt( 0, "lightskyblue" );
	grad.setColorAt( 1, "darkblue" );
	normalBrush = QBrush( grad );
	
	grad.setColorAt( 0, "lightpink" );
	grad.setColorAt( 1, "maroon" );
	selectionBrush = QBrush( grad );
	
	grad.setColorAt( 1, "#2A2AA5" );
	grad.setColorAt( 0, "#000060" );
	titleNormalBrush = QBrush( grad );
	
	grad.setColorAt( 1, "brown" );
	grad.setColorAt( 0, "#600000" );
	titleSelectionBrush = QBrush( grad );
	
	setPen( normalPen );
	setBrush( normalBrush );
}



void ClipViewItem::paint( QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget )
{
	QGraphicsRectItem::paint( painter, option, widget );
		
	QRectF inside = rect();
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
	}
}



void ClipViewItem::setSelected( bool b )
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



void ClipViewItem::mousePressEvent( QGraphicsSceneMouseEvent *event )
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



void ClipViewItem::mouseMoveEvent( QGraphicsSceneMouseEvent *event )
{
	if ( firstMove && fabs( event->scenePos().x() - moveStartMouse.x() ) < SNAPWIDTH )
		return;
	firstMove = false;
	Timeline* t = (Timeline*)scene();
	if ( moveResize )
		t->clipItemCanResize( this, moveResize, event->scenePos(), moveStartPosition, moveStartLength, moveStartMouse );
	else
		t->clipItemCanMove( this, event->scenePos(), moveStartPosition, moveStartMouse );
}



void ClipViewItem::mouseReleaseEvent( QGraphicsSceneMouseEvent *event )
{
	if ( firstMove )
		return;
	Timeline* t = (Timeline*)scene();
	if ( moveResize )
		t->clipItemResized( this, moveResize );
	else
		t->clipItemMoved( this, moveStartMouse );
}



void ClipViewItem::hoverMoveEvent( QGraphicsSceneHoverEvent *event )
{
	if ( event->pos().x() < SNAPWIDTH || event->pos().x() > rect().width() - SNAPWIDTH ) {
		setCursor( Qt::SplitHCursor );
	}
	else {
		setCursor( Qt::PointingHandCursor );
	}
}



void ClipViewItem::dragEnterEvent( QGraphicsSceneDragDropEvent *event )
{
	const QMimeData *mimeData = event->mimeData();
	if ( mimeData->formats().contains( MIMETYPEEFFECT ) ) {
		QString t = mimeData->data( MIMETYPEEFFECT ).data();
		event->accept();
	}
	else
		event->ignore();
}



void ClipViewItem::dropEvent( QGraphicsSceneDragDropEvent *event )
{
	const QMimeData *mimeData = event->mimeData();
	if ( mimeData->formats().contains( MIMETYPEEFFECT ) ) {
		QString t = mimeData->data( MIMETYPEEFFECT ).data();
		Timeline* tm = (Timeline*)scene();
		tm->addFilter( this, t );
		event->accept();
	}
}
