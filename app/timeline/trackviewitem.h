#ifndef TRACKVIEWITEM_H
#define TRACKVIEWITEM_H

#include "typerectitem.h"



class TrackViewItem : public TypeRectItem
{
public:
	TrackViewItem();

	void paint( QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget );
	
protected:
	void mousePressEvent( QGraphicsSceneMouseEvent * event );
};

#endif // TRACKVIEWITEM_H
