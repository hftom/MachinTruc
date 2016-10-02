#ifndef ABSTRACTVIEWITEM_H
#define ABSTRACTVIEWITEM_H

#include <QDateTime>
#include <QGraphicsRectItem>

#include "typeitem.h"



class AbstractViewItem : public QGraphicsRectItem
{
public:
	AbstractViewItem( double heightFactor = 1.0 );
	virtual ~AbstractViewItem() {}
	
	void setPosition( double pos );
	double getPosition() { return position; }
	void moveDelta( double d );
	void setLength( double len );
	double getLength() { return length; }
	void setGeometry( double pos, double len );
	void setCuts( double pos, double len, double scale );
	void setScale( double scale );
	
	virtual void setSelected( int i ) = 0;
	
protected:
	int selected;
	double position;
	double length;
	double scaleFactor;
	
private:
	void updateGeometry();
	
	int height;
};

#endif // ABSTRACTVIEWITEM_H
