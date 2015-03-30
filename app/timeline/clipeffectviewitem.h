#ifndef CLIPEFFECTVIEWITEM_H
#define CLIPEFFECTVIEWITEM_H

#include <QGraphicsSceneMouseEvent>
#include <QPainter>

#include "engine/clip.h"
#include "abstractviewitem.h"



class ClipEffectViewItem : public AbstractViewItem
{
public:
	ClipEffectViewItem( Clip *c, bool video, int id, double scale );
	virtual void setSelected( bool ) {}
	Clip* getClip() { return clip; }
	bool isVideoEffect() { return isVideo; }
	int getIndex() { return index; }
	
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
	
	Clip *clip;
	bool isVideo;
	int index;
};

#endif // CLIPEFFECTVIEWITEM_H
