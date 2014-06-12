#ifndef CURSORVIEWITEM_H
#define CURSORVIEWITEM_H

#include "typerectitem.h"



class CursorViewItem : public TypeRectItem
{
public:
	CursorViewItem();

	virtual void paint( QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget );
	
	void setHeight( double h );
};

#endif // CURSORVIEWITEM_H
