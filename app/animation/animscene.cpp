#include <QDebug>

#include "animscene.h"

#define DEFAULTZOOM 102400.0
#define ZOOMMAX 3276800.0
#define ZOOMMIN 800.0



AnimScene::AnimScene( AnimItem *a ) : QGraphicsScene()
{
	//zoom = DEFAULTZOOM;
	//viewWidth = 0;
	
	setBackgroundBrush( QBrush( QColor(255,255,255) ) );
	
	anim = a;
	addItem( anim );
}



void AnimScene::viewSizeChanged( const QSize &size )
{
	anim->setSize( size );
	setSceneRect( 0, 0, size.width(), size.height() );
	//viewWidth = size.width();
	//QTimer::singleShot ( 1, this, SLOT(updateLength()) );
}
