#ifndef AUDIOFILTER_H
#define AUDIOFILTER_H

#include "engine/filter.h"
#include "engine/frame.h"



class AudioFilter : public Filter
{
public:
	AudioFilter( QString id, QString name ) : Filter( id, name ) {}
	virtual ~AudioFilter() {}
	virtual bool process( Frame *src ) = 0;
};

#endif //AUDIOFILTER_H
