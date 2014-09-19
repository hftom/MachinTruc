#ifndef ABSTRACTVIEWITEM_H
#define ABSTRACTVIEWITEM_H

#include <QGraphicsRectItem>

#include "typeitem.h"



class AbstractViewItem : public QGraphicsRectItem
{
public:
	AbstractViewItem();
	
	void setPosition( double pos );
	double getPosition() { return position; }
	void setLength( double len );
	double getLength() { return length; }
	void setGeometry( double pos, double len );
	void setCuts( double pos, double len, double scale );
	void setScale( double scale );
	
	virtual void setSelected( bool b ) = 0;		
	
protected:
	bool selected;
	double position;
	double length;
	double scaleFactor;
	
private:
	void updateGeometry();
};

#endif // ABSTRACTVIEWITEM_H
