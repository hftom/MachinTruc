#ifndef TRANSITIONVIEWITEM_H
#define TRANSITIONVIEWITEM_H

#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include "abstractviewitem.h"



class TransitionViewItem : public AbstractViewItem
{
public:
	TransitionViewItem( QGraphicsItem *parent, double pos, double len, double scale );

	void paint( QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget );
	void setSelected( bool b ) { selected = b; }

private:	
	QPen normalPen, selectionPen;
	QBrush normalBrush, selectionBrush;
};

#endif // TRANSITIONVIEWITEM_H