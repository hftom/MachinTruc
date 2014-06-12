#ifndef CUT_H
#define CUT_H

#include "engine/source.h"



class Cut
{
public:
	Cut( Source *src, double st, double len );
	
	double getStart();
	double getLength();
	Source * getSource() const;

private:
	Source *source;
	double start, length;
};
#endif //CUT_H