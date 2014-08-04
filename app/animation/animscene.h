#ifndef ANIMSCENE_H
#define ANIMSCENE_H

#include <QGraphicsScene>

#include "animitem.h"



class AnimScene : public QGraphicsScene
{
	Q_OBJECT
public:
	explicit AnimScene( AnimItem *a );
	
public slots:
	void viewSizeChanged( const QSize &size );
	
private:
	AnimItem *anim;
	
signals:

};

#endif //ANIMSCENE_H