#ifndef KEYITEM_H
#define KEYITEM_H

#include <QGraphicsRectItem>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>

#define SNAPWIDTH 12
#define HALFKEYSIZE 5



class AnimItem;



class KeyItem : public QGraphicsRectItem
{
public:
	KeyItem( AnimItem *parent );
	void setSelected( bool b );
	
	void paint( QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget );
	
protected:
	void mousePressEvent( QGraphicsSceneMouseEvent *event );
	void mouseMoveEvent( QGraphicsSceneMouseEvent *event );
	void mouseReleaseEvent( QGraphicsSceneMouseEvent *event );
	void mouseDoubleClickEvent( QGraphicsSceneMouseEvent *event );
	
private:
	QPen normalPen, selectionPen;
	QBrush normalBrush, selectionBrush;
	bool selected;
	AnimItem *anim;
	
	QPointF moveStartPosition;
	QPointF moveStartMouse;
	bool firstMove;

};

#endif // KEYITEM_H
