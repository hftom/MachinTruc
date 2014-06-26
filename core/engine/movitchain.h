#ifndef MOVITCHAIN_H
#define MOVITCHAIN_H

#define GL_GLEXT_PROTOTYPES

#include <movit/effect_chain.h>
#include <movit/resource_pool.h>
#include <movit/effect.h>
#include <movit/input.h>

#include "engine/frame.h"
#include "vfx/glfilter.h"
#include "vfx/glcomposition.h"



using namespace movit;



class MovitInput
{
public:
	MovitInput();
	~MovitInput();

	bool process( Frame *src );
	Input* getMovitInput( Frame *src );

	static QString getDescriptor( Frame *src );

private:
	Input *input;
};



class MovitFilter
{
public:
	MovitFilter( const QList<Effect*> &el, GLFilter *f, bool owns = false );
	~MovitFilter();
	
	QList<Effect*> effects;
	GLFilter *filter;
	bool ownsFilter;
};



class MovitComposition
{
public:
	MovitComposition( Effect *e, GLComposition *c, bool owns = false );
	~MovitComposition();
	
	Effect *effect;
	GLComposition *composition;
	bool ownsComposition;
};



class MovitBranch
{
public:
	MovitBranch( MovitInput *in );
	~MovitBranch();
	
	MovitInput *input;
	QList<MovitFilter*> filters;
	MovitComposition *composition;
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
