#ifndef CLIPVIEWITEM_H
#define CLIPVIEWITEM_H

#include <QGraphicsSceneMouseEvent>

#include "engine/clip.h"
#include "transitionviewitem.h"

#define SNAPWIDTH 8



class ClipViewItem : public AbstractViewItem
{
public:
	ClipViewItem( Clip *c, double scale );
	~ClipViewItem();
	void updateTransition( double len );

	void paint( QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget );
	void setSelected( bool b );
	
	void setClip( Clip *c ) { clip = c; }
	Clip* getClip() { return clip; }
	
protected:
	void mousePressEvent( QGraphicsSceneMouseEvent *event );
	void mouseMoveEvent( QGraphicsSceneMouseEvent *event );
	void mouseReleaseEvent( QGraphicsSceneMouseEvent *event );
	void hoverMoveEvent( QGraphicsSceneHoverEvent *event );
	
	void dragEnterEvent( QGraphicsSceneDragDropEvent *event );
	void dropEvent( QGraphicsSceneDragDropEvent *event );

private:	
	QString filename;
	QPen normalPen, selectionPen;
	QBrush normalBrush, selectionBrush;
	QBrush titleNormalBrush, titleSelectionBrush;
	Clip *clip;
	
	double moveStartPosition;
	double moveStartLength;
	QPointF moveStartMouse;
	int moveResize;
	bool firstMove;
	bool multiMove;
	
	TransitionViewItem *transition;
};

#endif // CLIPVIEWITEM_H
