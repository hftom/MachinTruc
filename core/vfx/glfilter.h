#ifndef GLFILTER_H
#define GLFILTER_H

#define GL_GLEXT_PROTOTYPES

#include <movit/effect.h>
#include <movit/effect_util.h>

#include "engine/filter.h"
#include "engine/frame.h"

using namespace movit;



class GLFilter : public Filter
{
public:
	GLFilter( QString id, QString name ) : Filter( id, name ) {}
	virtual ~GLFilter() {}

	// single input effects
	virtual bool process( const QList<Effect*>&, double /*pts*/, Frame*, Profile* ) { return true; }
	// 2 inputs effects (transitions)
	virtual bool process( const QList<Effect*>&, double /*pts*/, Frame* /*first*/, Frame* /*second*/, Profile* ) { return true; }
	
	virtual QList<Effect*> getMovitEffects() = 0;
	virtual QList<Effect*> getMovitEffectsFirst() { QList<Effect*> list; return list; }
	virtual QList<Effect*> getMovitEffectsSecond() { QList<Effect*> list; return list; }

	virtual QString getDescriptor( double, Frame*, Profile* ) { return getIdentifier(); }
	virtual QString getDescriptorFirst( double, Frame*, Profile* ) { return ""; }
	virtual QString getDescriptorSecond( double, Frame*, Profile* ) { return ""; }
};

#endif //GLFILTER_H
