#include "abstractviewitem.h"



AbstractViewItem::AbstractViewItem()
	: selected( false ),
	position( 0 ),
	length( 0 ),
	scaleFactor( 1 )
{
}



void AbstractViewItem::setPosition( double pos )
{
	position = pos;
	updateGeometry();
}



void AbstractViewItem::moveDelta( double d )
{
	position += d;
	updateGeometry();
}



void AbstractViewItem::setLength( double len )
{
	length = len;
	updateGeometry();
}



void AbstractViewItem::setGeometry( double pos, double len )
{
	position = pos;
	length = len;
	updateGeometry();
}



void AbstractViewItem::setCuts( double pos, double len, double scale )
{
	position = pos;
	length = len;
	scaleFactor = scale;
	updateGeometry();
}



void AbstractViewItem::setScale( double scale )
{
	scaleFactor = scale;
	updateGeometry();
}



void AbstractViewItem::updateGeometry()
{
	double x = position / scaleFactor;
	double w = length / scaleFactor;
	double width = w - 1;
	if ( width < 0 )
		width = 0;
	setPos( x, 2 );
	setRect( 0, 0, width, TRACKVIEWITEMHEIGHT - 3 );
}
