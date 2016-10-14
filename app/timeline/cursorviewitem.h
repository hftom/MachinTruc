#ifndef CURSORVIEWITEM_H
#define CURSORVIEWITEM_H

#include <QGraphicsRectItem>
#include <QGraphicsSceneMouseEvent>



class CursorViewItem : public QGraphicsRectItem
{
public:
	CursorViewItem();

	virtual void paint( QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget );
	void setActiveTrack( int t );
	int getActiveTrack() { return activeTrack; }
	void setHeight( double h );
	
protected:
	void mousePressEvent( QGraphicsSceneMouseEvent *event );
	void mouseMoveEvent( QGraphicsSceneMouseEvent *event );
	void hoverMoveEvent( QGraphicsSceneHoverEvent *event );
	
private:
	double startMoveOffset;
	QPixmap trackMarker;
	int activeTrack;
	double markerYPos;
};

#endif // CURSORVIEWITEM_H
