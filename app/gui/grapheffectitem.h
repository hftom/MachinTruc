#ifndef GRAPHEFFECTITEM_H
#define GRAPHEFFECTITEM_H

#include <QGraphicsRectItem>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>

#include "engine/thumbnailer.h"
#include "graph.h"



class GraphEffectItem : public QGraphicsRectItem
{
public:
	GraphEffectItem( QString name, QString icon, int id );
	void paint( QPainter *painter, const QStyleOptionGraphicsItem*, QWidget* );
	void setSelected( bool b );
	bool isSelected() { return selected; }
	int index() { return filterIndex; }
	
protected:
	void mousePressEvent( QGraphicsSceneMouseEvent *event );
	void mouseMoveEvent( QGraphicsSceneMouseEvent *event );
	void mouseReleaseEvent( QGraphicsSceneMouseEvent *event );
	void hoverMoveEvent( QGraphicsSceneHoverEvent *event );
	
private:
	QImage image;
	QString text;
	int filterIndex;
	QPen textPen, backPen;
	QBrush backBrush, selectedBrush;
	
	bool selected;
	
	QDateTime lastTime;
};

#endif // GRAPHEFFECTITEM_H