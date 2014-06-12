#include "abstractviewitem.h"



AbstractViewItem::AbstractViewItem( int t) : TypeRectItem( t )
{
	position = length = 0;
	selected = false;
};



void AbstractViewItem::setCuts( double pos, double len )
{
	position = pos;
	length = len;
}



void AbstractViewItem::setPosition( double pos, double scale )
{
	position = pos;
	setScale( scale );
}



void AbstractViewItem::setLength( double len, double scale )
{
	length = len;
	setScale( scale );
}



void AbstractViewItem::setScale( double d )
{
	double x = position / d;
	double w = length / d;
	double width = w - 1;
	if ( width < 0 )
		width = 0;
	setPos( x, 2 );
	setRect( 0, 0, width, TRACKVIEWITEMHEIGHT - 3 );
}
