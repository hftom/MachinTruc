#ifndef CURSORVIEWITEM_H
#define CURSORVIEWITEM_H

#include <QGraphicsRectItem>



class CursorViewItem : public QGraphicsRectItem
{
public:
	CursorViewItem();

	virtual void paint( QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget );
	
	void setHeight( double h );
};

#endif // CURSORVIEWITEM_H
