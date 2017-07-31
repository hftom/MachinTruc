#ifndef GLOPACITY_H
#define GLOPACITY_H

#include "vfx/glfilter.h"



class GLOpacity : public GLFilter
{
public:
	GLOpacity( QString id, QString name );
	~GLOpacity();

	bool process( const QList<Effect*> &el, double pts, Frame *src, Profile *p );

	QList<Effect*> getMovitEffects();

protected:
	Parameter *factor;
};



class GLFadeIn : public GLOpacity
{
public:
	GLFadeIn( QString id, QString name ) : GLOpacity( id, name ) {
		setSnap( SNAPSTART );
		setLength( MICROSECOND * 2 );
		factor->graph.keys.append( AnimationKey( AnimationKey::CURVE, 0, 0 ) );
		factor->graph.keys.append( AnimationKey( AnimationKey::CURVE, 1, 1 ) );
		factor->hidden = true;
	}
};



class GLFadeOut : public GLOpacity
{
public:
	GLFadeOut( QString id, QString name ) : GLOpacity( id, name ) {
		setSnap( SNAPEND );
		setLength( MICROSECOND * 2 );
		factor->graph.keys.append( AnimationKey( AnimationKey::CURVE, 0, 1 ) );
		factor->graph.keys.append( AnimationKey( AnimationKey::CURVE, 1, 0 ) );
		factor->hidden = true;
	}
};

#endif //GLOPACITY_H
