#ifndef CLIPEFFECTVIEWITEM_H
#define CLIPEFFECTVIEWITEM_H

#include <QGraphicsSceneMouseEvent>
#include <QPainter>

#include "abstractviewitem.h"



class ClipEffectViewItem : public AbstractViewItem
{
public:
	ClipEffectViewItem();
	virtual void setSelected( bool b ) {}	
	
protected:
	void mousePressEvent( QGraphicsSceneMouseEvent *event );
	void mouseMoveEvent( QGraphicsSceneMouseEvent *event );
	void mouseReleaseEvent( QGraphicsSceneMouseEvent *event );
	void hoverMoveEvent( QGraphicsSceneHoverEvent *event );

private:	
	QPen normalPen;
	QBrush normalBrush;
	
	double moveStartPosition;
	double moveStartLength;
	QPointF moveStartMouse;
	int moveResize;
	bool firstMove;
};

#endif // CLIPEFFECTVIEWITEM_H
