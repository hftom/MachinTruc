#include <math.h>

#include <QCursor>
#include <QMenu>

#include "engine/filtercollection.h"
#include "timeline.h"
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
}



void TransitionViewItem::mousePressEvent( QGraphicsSceneMouseEvent *event )
{
	if ( event->buttons() & Qt::RightButton ) {
		FilterCollection *fc = FilterCollection::getGlobalInstance();
		QMenu menu;
		for ( int i = 0; i < fc->videoTransitions.count(); ++i ) {
			menu.addAction( fc->videoTransitions[ i ].name );
		}
		QAction *action = menu.exec( QCursor::pos() );
		if ( action ) {
			for ( int i = 0; i < fc->videoTransitions.count(); ++i ) {
				if ( action->text() == fc->videoTransitions[ i ].name ) {
					Timeline *t = (Timeline*)scene();
					t->transitionChanged( this, fc->videoTransitions[ i ].name );
					break;
				}
			}
		}
		
		event->accept();
	}
	
	event->ignore();
}
