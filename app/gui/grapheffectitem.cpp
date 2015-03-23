#include <QApplication>

#include "grapheffectitem.h"


#define NORMALZ 1
#define SELECTEDZ 2




GraphEffectItem::GraphEffectItem( QString name, QString icon, int id ) : QGraphicsRectItem()
{
	image = QImage( QString(":/images/icons/%1.png").arg( icon ) );
	text = name;
	filterIndex = id;
	textPen.setColor( "white" );
	backPen.setColor( QColor(0,0,0,0) );
	backBrush = QBrush( QColor(0,0,0,180) );
	selectedBrush = QBrush( QColor(128,0,0,180) );
	setRect( 0, 0, ICONSIZEWIDTH, ICONSIZEHEIGHT );
	selected = false;
}



void GraphEffectItem::paint( QPainter *painter, const QStyleOptionGraphicsItem*, QWidget* )
{
	QRectF inside = rect();
	painter->drawImage( inside, image );
	painter->setPen( backPen );
	if ( selected )
		painter->setBrush( selectedBrush );
	else
		painter->setBrush( backBrush );
	painter->drawRect( inside );
	painter->setPen( textPen );
	painter->drawText( inside, Qt::AlignCenter | Qt::TextWordWrap, text );
}



void GraphEffectItem::setSelected( bool b )
{
	selected = b;
	if ( selected ) {
		setZValue( SELECTEDZ );
	}
	else {
		setZValue( NORMALZ );
	}
	update();
}



void GraphEffectItem::mousePressEvent( QGraphicsSceneMouseEvent *event )
{
	Graph* g = (Graph*)scene();
	
	if ( event->buttons() & Qt::LeftButton ) {
		QDateTime old = lastTime;
		lastTime = QDateTime::currentDateTime();
		if ( old.msecsTo( lastTime ) <= qApp->doubleClickInterval() ) {
			//g->effectDoubleClicked( this );
			//return;
		}
	}
	else if ( event->buttons() & Qt::RightButton ) {
		g->effectRightClick( this );
		return;
	}

	g->itemSelected( this );
}



void GraphEffectItem::mouseMoveEvent( QGraphicsSceneMouseEvent *event )
{
	Graph* g = (Graph*)scene();
	//g->effectMoved( this, event->scenePos(), moveStartPosition, moveStartMouse, unsnap, multiMove );
}



void GraphEffectItem::mouseReleaseEvent( QGraphicsSceneMouseEvent * )
{
	Graph* g = (Graph*)scene();
	//g->effectReleased( this, moveStartMouse, multiMove );
}



void GraphEffectItem::hoverMoveEvent( QGraphicsSceneHoverEvent * )
{
	setCursor( QCursor(Qt::PointingHandCursor) );
}
