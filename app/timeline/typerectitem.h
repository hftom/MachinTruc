#ifndef TYPERECTITEM_H
#define TYPERECTITEM_H

#include <QGraphicsRectItem>

#define TRACKVIEWITEMHEIGHT 37



class TypeRectItem : public QGraphicsRectItem
{
public:
	enum Type{ UNDEF, CURSOR, VIDEOCUT, VIDEOFILTER, AUDIOCUT, AUDIOFILTER };
	
	TypeRectItem( int t );
	int getItemType() { return type; }

private:
	int type;
};

#endif // TYPERECTITEM_H
