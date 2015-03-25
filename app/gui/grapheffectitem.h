#ifndef GRAPHEFFECTITEM_H
#define GRAPHEFFECTITEM_H

#include <QGraphicsRectItem>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>

#include "engine/thumbnailer.h"
#include "graph.h"



class GraphItem : public QGraphicsRectItem
{
public:
	GraphItem( bool fx ) : QGraphicsRectItem(), isEffect( fx ) {}
	
	bool isEffect;
};



class GraphThumb : public GraphItem
{
public:
	GraphThumb( QImage &img ) : GraphItem( false ), thumb( img ) {
		setRect( 0, 0, ICONSIZEWIDTH, ICONSIZEHEIGHT );
	}
	void paint( QPainter *painter, const QStyleOptionGraphicsItem*, QWidget* ) {
		QRectF inside = rect();
		painter->drawImage( inside, thumb );
		painter->setPen( "black" );
		painter->drawRect( inside );
	}
	
private:
	QImage thumb;
};



class GraphEffectItem : public GraphItem
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
	
private:
	QImage image;
	QString text;
	int filterIndex;
	QPen textPen, backPen;
	QBrush backBrush, selectedBrush;
	
	bool selected;
	
	QDateTime lastTime;
	bool firstMove;
	qreal mouseOffset, moveStart;
};

#endif // GRAPHEFFECTITEM_H