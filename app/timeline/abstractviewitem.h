#ifndef ABSTRACTVIEWITEM_H
#define ABSTRACTVIEWITEM_H

#include "typerectitem.h"



class AbstractViewItem : public TypeRectItem
{
public:
	AbstractViewItem( int t);
	
	void setCuts( double pos, double len );
	
	double getPosition() { return position; }
	double getLength() { return length; }
	void setPosition( double pos, double scale );
	void setLength( double len, double scale );
	void setScale( double d );
	
	virtual void setSelected( bool b ) = 0;		
	
protected:
	double position;
	double length;
	bool selected;
};

#endif // ABSTRACTVIEWITEM_H
