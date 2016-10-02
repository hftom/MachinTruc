#ifndef CLIPVIEWITEM_H
#define CLIPVIEWITEM_H

#include <QGraphicsSceneMouseEvent>

#include "engine/clip.h"
#include "engine/thumbnailer.h"
#include "transitionviewitem.h"



class ClipViewItem : public AbstractViewItem
{
public:
	ClipViewItem( Clip *c, double scale );
	~ClipViewItem();
	void updateTransition( double len );

	void paint( QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget );
	void setSelected( int i );
	bool setThumb( ThumbRequest res );
	QImage& getStartThumb() { return startThumb; }
	
	void setClip( Clip *c ) { clip = c; }
	Clip* getClip() { return clip; }
	
	TransitionViewItem* getTransition() { return transition; }
	
protected:
	void mousePressEvent( QGraphicsSceneMouseEvent *event );
	void mouseMoveEvent( QGraphicsSceneMouseEvent *event );
	void mouseReleaseEvent( QGraphicsSceneMouseEvent *event );
	void hoverMoveEvent( QGraphicsSceneHoverEvent *event );
	
	void dragEnterEvent( QGraphicsSceneDragDropEvent *event );
	void dropEvent( QGraphicsSceneDragDropEvent *event );

private:	
	QString filename;
	QPen normalPen, selectionPen, currentPen;
	QBrush normalBrush, selectionBrush, currentBrush;
	QBrush titleNormalBrush, titleSelectionBrush, titleCurrentBrush;
	Clip *clip;
	
	double moveStartPosition;
	double moveStartLength;
	QPointF moveStartMouse;
	int moveResize;
	bool firstMove;
	bool multiMove;
	
	QImage startThumb, endThumb;
	
	TransitionViewItem *transition;
	
	QDateTime lastTime;
};

#endif // CLIPVIEWITEM_H
