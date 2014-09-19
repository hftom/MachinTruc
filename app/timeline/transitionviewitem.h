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
	
protected:
	/*void mousePressEvent( QGraphicsSceneMouseEvent *event );
	void mouseMoveEvent( QGraphicsSceneMouseEvent *event );
	void mouseReleaseEvent( QGraphicsSceneMouseEvent *event );
	void hoverMoveEvent( QGraphicsSceneHoverEvent *event );
	
	void dragEnterEvent( QGraphicsSceneDragDropEvent *event );
	void dropEvent( QGraphicsSceneDragDropEvent *event );*/

private:	
	QPen normalPen, selectionPen;
	QBrush normalBrush, selectionBrush;
	
	double moveStartPosition;
	double moveStartLength;
	QPointF moveStartMouse;
	int moveResize;
	bool firstMove;
};

#endif // TRANSITIONVIEWITEM_H
