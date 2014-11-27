#ifndef AUDIOFILTER_H
#define AUDIOFILTER_H

#include "engine/filter.h"
#include "engine/frame.h"



class AudioFilter : public Filter
{
public:
	AudioFilter( QString id, QString name ) : Filter( id, name ) {}
	virtual ~AudioFilter() {}
	// filters
	virtual bool process( Frame*, Buffer* /*src*/, Buffer* /*dst*/, Profile* ) { return true; }
	// transitions
	virtual bool process( Frame* /*first*/, Buffer* /*fisrt*/, Frame* /*second*/, Buffer* /*second*/, Buffer* /*dest*/, Profile* ) { return true; }
};

#endif //AUDIOFILTER_H
