#ifndef AUDIOCOMPOSITION_H
#define AUDIOCOMPOSITION_H

#include "engine/frame.h"



class AudioComposition : public QObject
{
	Q_OBJECT
public:
	AudioComposition() {
		valid = false;
		posInTrack = 0;
		len = 0;
	}
	virtual ~AudioComposition() {}

	virtual bool process( Frame *src, Frame *dst ) = 0;
	
	bool isValid() { return valid; }

public slots:
	virtual QStringList getProperties()	{ return QStringList(); }
	virtual void setProperty( QStringList /*properties*/ ) {}
	
	void setPosition( double p ) { posInTrack = p; }
	double position() { return posInTrack; }
	void setLength( double l ) { len = l; }
	double length() { return len; }

protected:
	bool valid;
	double posInTrack, len;
};

#endif //AUDIOCOMPOSITION_H
