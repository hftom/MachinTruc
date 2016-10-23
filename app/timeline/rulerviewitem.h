#ifndef RULERVIEWITEM_H
#define RULERVIEWITEM_H

#include <QPropertyAnimation>
#include <QPainter>
#include <QGraphicsRectItem>
#include <QGraphicsSceneMouseEvent>
#include <QTimer>
#include <QPen>
#include <QBrush>



class RulerViewItem : public QObject, public QGraphicsRectItem
{
	Q_OBJECT
	Q_PROPERTY( qreal posy READ y WRITE setY )
public:
	RulerViewItem();
	
	void setPosition( qreal posx, qreal posy );
	void setTimeScale( double pps );
	void setFrameDuration( double d ) { frameDuration = d; }
	void dock() { docked = true; }
	void undock() { docked = false; }
	bool isDocked() { return docked; }

	virtual void paint( QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget );
	
private:
	QLinearGradient getTextGradient( double start, double textLen, bool revert );
	
	QPixmap background;
	QFont font;
	double frameDuration;
	double pixelsPerUnit; // second or frame
	QPropertyAnimation *anim;
	qreal lastY;
	int ticklen;
	double tickdistance;
	int textShortLen, textLongLen, currentTextLen;
	
	bool docked;
};

#endif // RULERVIEWITEM_H