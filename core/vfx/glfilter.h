#ifndef GLFILTER_H
#define GLFILTER_H

#define GL_GLEXT_PROTOTYPES

#include <movit/effect.h>

#include "engine/filter.h"
#include "engine/frame.h"

using namespace movit;



class GLFilter : public Filter
{
	Q_OBJECT
public:
	GLFilter( QString id, QString name ) : Filter( id, name ) {
		posInTrack = 0;
		length = 0;
	}
	virtual ~GLFilter() {}
	virtual bool preProcess( Frame*, Profile* ) { return true; }
	virtual bool process( const QList<Effect*>&, Frame*, Profile* ) { return true; }
	virtual QList<Effect*> getMovitEffects() = 0;

	virtual QString getDescriptor() { return getIdentifier(); }

public slots:
	void setPosition( double p ) { posInTrack = p; }
	double getPosition() { return posInTrack; }
	void setLength( double len ) { length = len; }
	double getlength() { return length; }

protected:
	double posInTrack, length;
};

#endif //GLFILTER_H
