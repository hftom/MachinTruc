#ifndef CLIPVIEWITEM_H
#define CLIPVIEWITEM_H

#include <QGraphicsSceneMouseEvent>

#include "engine/clip.h"
#include "abstractviewitem.h"

#define SNAPWIDTH 8



class ClipViewItem : public AbstractViewItem
{
public:
	ClipViewItem( Clip *c, double scale );

	void paint( QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget );
	void setSelected( bool b );
	
	void setClip( Clip *c ) { clip = c; }
	Clip* getClip() { return clip; }
	
protected:
	void mousePressEvent( QGraphicsSceneMouseEvent * event );
	void mouseMoveEvent( QGraphicsSceneMouseEvent * event );
	void mouseReleaseEvent( QGraphicsSceneMouseEvent * event );
	void hoverMoveEvent( QGraphicsSceneHoverEvent * event );

private:	
	QString filename;
	QPen normalPen, selectionPen;
	QBrush normalBrush, selectionBrush;
	Clip *clip;
	
	double moveStartPosition;
	double moveStartLength;
	QPointF moveStartMouse;
	int moveResize;
	bool firstMove;
};

#endif // CLIPVIEWITEM_H
