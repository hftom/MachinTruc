#ifndef AUDIOFILTER_H
#define AUDIOFILTER_H

#include "engine/filter.h"
#include "engine/frame.h"



class AudioFilter : public Filter
{
	Q_OBJECT
public:
	AudioFilter( QString id, QString name ) : Filter( id, name ) {
		posInTrack = 0;
		length = 0;
	}
	virtual ~AudioFilter() {}
	virtual bool process( Frame *src ) = 0;

public slots:
	void setPosition( double p ) { posInTrack = p; }
	double getPosition() { return posInTrack; }
	void setLength( double len ) { length = len; }
	double getLength() { return length; }

protected:
	double posInTrack, length;
};

#endif //AUDIOFILTER_H
