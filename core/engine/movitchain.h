#ifndef MOVITCHAIN_H
#define MOVITCHAIN_H

#define GL_GLEXT_PROTOTYPES

#include <movit/effect_chain.h>
#include <movit/resource_pool.h>
#include <movit/effect.h>
#include <movit/input.h>

#include "engine/frame.h"
#include "vfx/glfilter.h"



using namespace movit;



class MovitInput
{
public:
	MovitInput();
	~MovitInput();

	bool process( Frame *src, GLResource *gl = NULL );
	Input* getMovitInput( Frame *src );

	static QString getDescriptor( Frame *src );

private:
	bool setBuffer( PBO *p, Frame *src, int size );
	Input *input;
	qint64 mmi;
};



class MovitFilter
{
public:
	MovitFilter( const QList<Effect*> &el, GLFilter *f = NULL );
	
	QList<Effect*> effects;
	QSharedPointer<GLFilter> filter;
};



class MovitBranch
{
public:
	MovitBranch( MovitInput *in );
	~MovitBranch();
	
	MovitInput *input;
	QList<MovitFilter*> filters;
	MovitFilter *overlay;
};



class MovitChain
{
public:
	MovitChain();	
	~MovitChain();
	void reset();
	
	EffectChain *chain;
	QList<MovitBranch*> branches;
};

#endif //MOVITCHAIN_H
