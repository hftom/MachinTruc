#ifndef GLFILTER_H
#define GLFILTER_H

#define GL_GLEXT_PROTOTYPES

#include <movit/effect.h>

#include "engine/filter.h"
#include "engine/frame.h"

using namespace movit;



class GLFilter : public Filter
{
public:
	GLFilter( QString id, QString name ) : Filter( id, name ) {}
	virtual ~GLFilter() {}
	virtual bool preProcess( Frame*, Profile* ) { return true; }
	virtual bool process( const QList<Effect*>&, Frame*, Profile* ) { return true; }
	virtual QList<Effect*> getMovitEffects() = 0;

	virtual QString getDescriptor() { return getIdentifier(); }
};

#endif //GLFILTER_H
