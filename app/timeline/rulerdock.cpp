#include <QPainter>

#include "timeline.h"
#include "typeitem.h"
#include "rulerdock.h"



RulerDock::RulerDock()
{
	setData( DATAITEMTYPE, TYPERULERDOCK );
	setRect( 0, 0, 1, RULERDOCKHEIGHT );
	
	QLinearGradient grad( QPointF(0, 0), QPointF(0, 1) );
	grad.setCoordinateMode( QGradient::ObjectBoundingMode );
	grad.setColorAt( 0, "#606020" );
	grad.setColorAt( 1, "#B0B050" );

	setPen( QColor( "#606020" ) );
	setBrush( QBrush(grad) );
}



void RulerDock::mousePressEvent( QGraphicsSceneMouseEvent * event )
{
	Timeline *t = (Timeline*)scene();
	if ( event->button() == Qt::LeftButton ) {
		t->trackPressed( event->scenePos() );
	}
}