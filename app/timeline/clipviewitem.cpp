#include <math.h>

#include <QApplication>
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
	filename = clip->getSource()->getDisplayName();
	
	setCuts( clip->position(), clip->length(), scale );

	normalPen.setJoinStyle( Qt::MiterJoin );
	normalPen.setColor( "blue" );
	
	selectionPen.setJoinStyle( Qt::MiterJoin );
	selectionPen.setColor( "purple" );
	
	currentPen.setJoinStyle( Qt::MiterJoin );
	currentPen.setColor( "red" );

	QLinearGradient grad( QPointF(0, 0), QPointF(0, 1) );
	grad.setCoordinateMode( QGradient::ObjectBoundingMode );
	grad.setColorAt( 0, QColor(135,206,250,180) );
	grad.setColorAt( 1, QColor(0,0,89,180) );
	normalBrush = QBrush( grad );
	
	grad.setColorAt( 0, QColor(193,182,255,180) );
	grad.setColorAt( 1, QColor(44,0,44,180) );
	selectionBrush = QBrush( grad );
	
	grad.setColorAt( 0, QColor(255,182,193,180) );
	grad.setColorAt( 1, QColor(89,0,0,180) );
	currentBrush = QBrush( grad );
	
	grad.setColorAt( 1, "#2A2AA5" );
	grad.setColorAt( 0, "#000060" );
	titleNormalBrush = QBrush( grad );

	grad.setColorAt( 1, "#A000A0" );
	grad.setColorAt( 0, "#300030" );
	titleSelectionBrush = QBrush( grad );
	
	grad.setColorAt( 1, "brown" );
	grad.setColorAt( 0, "#600000" );
	titleCurrentBrush = QBrush( grad );
	
	setPen( normalPen );
	setBrush( normalBrush );
	
	lastTime = QDateTime::currentDateTime();
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



bool ClipViewItem::setThumb( ThumbRequest res )
{
	bool isStart = false;
	if ( res.thumbPTS == clip->start() ) {
		if ( clip->getSpeed() < 0 )
			endThumb = res.thumb;
		else {
			isStart = true;
			startThumb = res.thumb;
		}
	}
	else if ( res.thumbPTS == clip->start() + (clip->length() * qAbs(clip->getSpeed())) - clip->getProfile().getVideoFrameDuration() ) {
		if ( clip->getSpeed() < 0 ) {
			isStart = true;
			startThumb = res.thumb;
		}
		else
			endThumb = res.thumb;
	}
	update();
	
	return isStart;
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
#if QT_VERSION < 0x050000
	inside.setWidth( inside.width() + 1 );
#endif
	inside.setHeight( inside.height() + 1 );
	qreal w = inside.width();
	if ( !endThumb.isNull() && w > 1 ) {
		QRectF src = endThumb.rect();
		QRectF ir( inside ); 
		ir.setWidth( ir.height() * src.width() / src.height() );
		if ( ir.width() > w ) {
			src.setX( (ir.width() - w) * src.height() / src.height() );
			painter->drawImage( inside, endThumb, src );
		}
		else {
			ir.moveLeft( w - ir.width() );
			painter->drawImage( ir, endThumb );
		}
	}
	if ( !startThumb.isNull() && w > 1 ) {
		QRectF src = startThumb.rect();
		QRectF ir( inside ); 
		ir.setWidth( ir.height() * src.width() / src.height() );
		if ( ir.width() > w ) {
			src.setWidth( src.width() - (ir.width() - w) * src.height() / ir.height() );
			painter->drawImage( inside, startThumb, src );
		}
		else
			painter->drawImage( ir, startThumb );
	}
	
	// draw title
	painter->setPen( QColor(0,0,0,0) );
	if (selected == 2) {
		painter->setBrush( titleCurrentBrush );
	}
	else if ( selected == 1)
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



void ClipViewItem::setSelected( int i )
{
	selected = i;
	if ( selected == 2 ) {
		setZValue( ZCLIPCURRENT );
		setBrush( currentBrush );
		setPen( currentPen );
	}
	else if ( selected == 1 ) {
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
	if ( event->buttons() & Qt::LeftButton ) {
		QDateTime old = lastTime;
		lastTime = QDateTime::currentDateTime();
		if ( old.msecsTo( lastTime ) <= qApp->doubleClickInterval() ) {
			Timeline *t = (Timeline*)scene();
			t->clipDoubleClicked();
		}
	}
	else if ( event->buttons() & Qt::RightButton ) {
		Timeline* t = (Timeline*)scene();
		t->clipRightClick( this );
	}
	
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
	t->itemSelected( this, event->modifiers() & Qt::ControlModifier );
}



void ClipViewItem::mouseMoveEvent( QGraphicsSceneMouseEvent *event )
{
	bool unsnap = event->modifiers() & Qt::ControlModifier;
	if ( firstMove && !unsnap && fabs( event->scenePos().x() - moveStartMouse.x() ) < SNAPWIDTH )
		return;
	
	Timeline* t = (Timeline*)scene();
	if ( firstMove )
		t->undockRuler();
	firstMove = false;
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
	
	t->dockRuler();
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
