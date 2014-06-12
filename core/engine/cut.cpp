#include "engine/cut.h"



Cut::Cut( Source *src, double st, double len )
{
	source = src;
	start = st;
	length = len;
}



double Cut::getStart()
{
	return start;
}



double Cut::getLength()
{
	return length;
}



Source * Cut::getSource() const
{
	return source;
}