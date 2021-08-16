#ifndef GLCUT_H
#define GLCUT_H

#include <movit/effect_util.h>

#include "vfx/glmask.h"



class MyCutEffect : public Effect {
public:
	MyCutEffect() {}
	std::string effect_type_id() const { return "MyCutEffect"; }
	std::string output_fragment_shader() { return GLFilter::getShader("cut.frag"); }
};



class GLCut : public GLMask
{
	Q_OBJECT
public:
	GLCut( QString id, QString name );
	~GLCut();
	
	bool process( const QList<Effect*> &el, double pts, Frame *src, Profile *p );
	QString getDescriptor( double pts, Frame *src, Profile *p  );

	QList<Effect*> getMovitEffects();
};

#endif // GLCUT_H
