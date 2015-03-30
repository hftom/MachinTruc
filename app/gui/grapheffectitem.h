#ifndef GRAPHEFFECTITEM_H
#define GRAPHEFFECTITEM_H

#include <QApplication>
#include <QGraphicsRectItem>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>

#include "engine/thumbnailer.h"
#include "graph.h"



class GraphItem : public QGraphicsRectItem
{
public:
	GraphItem( bool fx ) : QGraphicsRectItem(), isEffect( fx ), selected( false ) {}
	bool isSelected() { return selected; }
	
	bool isEffect;
	
protected:
	QDateTime lastTime;
	bool selected;
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
		if ( selected )
			painter->setPen( QPen( QBrush(QColor("red")), 2 ) );
		else
			painter->setPen( QPen( QBrush(QColor("black")), 1 ) );
		painter->drawRect( inside );
	}
	void setSelected( bool b ) {
		selected = b;
		update();
	}
	
protected:
	void mousePressEvent( QGraphicsSceneMouseEvent *event ) {
		if ( event->buttons() & Qt::LeftButton ) {
			QDateTime old = lastTime;
			lastTime = QDateTime::currentDateTime();
			if ( old.msecsTo( lastTime ) <= qApp->doubleClickInterval() ) {
				Graph* g = (Graph*)scene();
				g->itemDoubleClicked();
			}
		}
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
	
	bool firstMove;
	qreal mouseOffset, moveStart;
};

#endif // GRAPHEFFECTITEM_H