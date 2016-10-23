#ifndef RULERDOCK_H
#define RULERDOCK_H

#include <QGraphicsRectItem>



class RulerDock : public QGraphicsRectItem
{
public:
	RulerDock();
	
protected:
	void mousePressEvent( QGraphicsSceneMouseEvent * event );
};

#endif // RULERDOCK_H
