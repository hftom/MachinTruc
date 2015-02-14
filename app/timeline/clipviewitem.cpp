#include <math.h>

#include <QFileInfo>
#include <QMimeData>
#include <QCursor>

#include "gui/mimetypes.h"
#include "timeline.h"
#include "clipviewitem.h"

#include "clipeffectviewitem.h"



ClipViewItem::ClipViewItem( Clip *c, double scale ) : AbstractViewItem(),
	moveStartPosition( 0 ),
	moveStartLength( 0 ),
	moveResize( 0 ),
	firstMove( true ),
	multiMove( false ),
	transition( NULL )
{
	setData( DATAITEMTYPE, TYPECLIP );
	setZValue( ZCLIP );
	//setFlag( QGraphicsItem::ItemIsMovable, true );
	//setFlag(QGraphicsItem::ItemIsSelectable, true);
	//setFlag(QGraphicsItem::ItemIsFocusable, true);
	setAcceptHoverEvents(true);
	setAcceptDrops( true );
	 
	setClip( c );
	filename = clip->sourcePath();
	
	setCuts( clip->position(), clip->length(), scale );

	normalPen.setJoinStyle( Qt::MiterJoin );
	normalPen.setColor( "blue" );
	
	selectionPen.setJoinStyle( Qt::MiterJoin );
	selectionPen.setColor( "red" );

	QLinearGradient grad( QPointF(0, 0), QPointF(0, 1) );
	grad.setCoordinateMode( QGradient::ObjectBoundingMode );
	//grad.setColorAt( 0, "lightskyblue" );
	//grad.setColorAt( 1, "darkblue" );
	grad.setColorAt( 0, QColor(135,206,250,180) );
	grad.setColorAt( 1, QColor(0,0,89,180) );
	normalBrush = QBrush( grad );
	
	grad.setColorAt( 0, QColor(255,182,193,180) );
	grad.setColorAt( 1, QColor(89,0,0,180) );
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



ClipViewItem::~ClipViewItem()
{
	/*if ( transition ) {
		delete transition;
	}*/
}



void ClipViewItem::updateTransition( double len )
{
	if ( len == 0 ) {
		if ( transition ) {
			delete transition;
			transition = NULL;
		}
	}
	else {
		if ( !transition ) {
			transition = new TransitionViewItem( parentItem(), position, len, scaleFactor );
		}
		else
			transition->setGeometry( position, len );
	}
}



void ClipViewItem::setThumb( ThumbRequest res )
{
	if ( res.thumbPTS == clip->start() ) {
		startThumb = res.thumb;
	}
	else if ( res.thumbPTS == clip->start() + clip->length() - clip->getProfile().getVideoFrameDuration() ) {
		endThumb = res.thumb;
	}
	update();
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

	// draw thumbs
	inside.setWidth( inside.width() + 1 );
	inside.setHeight( inside.height() + 1 );
	qreal w = inside.width();
	if ( !endThumb.isNull() && w > 1 ) {
		QRectF th = endThumb.rect();
		QRectF ir( inside ); 
		ir.setWidth( ir.height() * th.width() / th.height() );
		if ( ir.width() > w ) {
			th.setX( (ir.width() - w) * th.height() / ir.height() );
			painter->drawImage( inside, endThumb, th );
		}
		else {
			ir.moveLeft( w - ir.width() );
			painter->drawImage( ir, endThumb );
		}
	}
	if ( !startThumb.isNull() && w > 1 ) {
		QRectF th = startThumb.rect();
		QRectF ir( inside ); 
		ir.setWidth( ir.height() * th.width() / th.height() );
		if ( ir.width() > w ) {
			th.setWidth( th.width() - (ir.width() - w) * th.height() / ir.height() );
			painter->drawImage( inside, startThumb, th );
		}
		else
			painter->drawImage( ir, startThumb );
	}
	
	// draw title
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
		setZValue( ZCLIPSELECTED );
		setBrush( selectionBrush );
		setPen( selectionPen );
	}
	else {
		setZValue( ZCLIP );
		setPen( normalPen );
		setBrush( normalBrush );
	}
	update();
}



void ClipViewItem::mousePressEvent( QGraphicsSceneMouseEvent *event )
{
	firstMove = true;
	multiMove = event->modifiers() & Qt::ShiftModifier;
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
	bool unsnap = event->modifiers() & Qt::ControlModifier;
	if ( firstMove && !unsnap && fabs( event->scenePos().x() - moveStartMouse.x() ) < SNAPWIDTH )
		return;
	firstMove = false;
	Timeline* t = (Timeline*)scene();
	if ( moveResize )
		t->clipItemCanResize( this, moveResize, event->scenePos(), moveStartPosition, moveStartLength, moveStartMouse, unsnap );
	else
		t->clipItemCanMove( this, event->scenePos(), moveStartPosition, moveStartMouse, unsnap, multiMove );
}



void ClipViewItem::mouseReleaseEvent( QGraphicsSceneMouseEvent *event )
{
	Q_UNUSED( event );
	
	if ( firstMove )
		return;
	Timeline* t = (Timeline*)scene();
	if ( moveResize )
		t->clipItemResized( this, moveResize );
	else
		t->clipItemMoved( this, moveStartMouse, multiMove );
}



void ClipViewItem::hoverMoveEvent( QGraphicsSceneHoverEvent *event )
{
	if ( event->pos().x() < SNAPWIDTH || event->pos().x() > rect().width() - SNAPWIDTH ) {
		setCursor( QCursor(Qt::SplitHCursor) );
	}
	else {
		setCursor( QCursor(Qt::PointingHandCursor) );
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
