#ifndef GLFADEOUTIN_H
#define GLFADEOUTIN_H

#include "vfx/glfilter.h"



class MyFadeOutInEffect : public Effect {
public:
	MyFadeOutInEffect() : show_second( 0 ) {
		register_float( "show_second", &show_second );
		register_float( "opacity", &opacity );
	}	
	virtual std::string effect_type_id() const { return "MyFadeOutInEffect"; }
	std::string output_fragment_shader() { return GLFilter::getShader("fade_out_in.frag"); }
	virtual unsigned num_inputs() const { return 2; }

private:
	float show_second;
	float opacity;
};



class GLFadeOutIn : public GLFilter
{
	Q_OBJECT
public:
	GLFadeOutIn( QString id, QString name );

	bool process( const QList<Effect*>&, double pts, Frame *first, Frame *second, Profile *p );
	QList<Effect*> getMovitEffects();

protected:
	Parameter *pause;
};

#endif //GLFADEOUTIN_H
