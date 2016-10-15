#ifndef SELECTWINDOWITEM_H
#define SELECTWINDOWITEM_H

#include <QGraphicsRectItem>



class SelectWindowItem : public QGraphicsRectItem
{
public:
	SelectWindowItem( QPointF start );
	void setEndPoint( QPointF end );
	
private:
	double startX, startY;
};

#endif // SELECTWINDOWITEM_H
