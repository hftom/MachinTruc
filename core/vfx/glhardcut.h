#ifndef GLHARDCUT_H
#define GLHARDCUT_H

#include "vfx/glfilter.h"



static const char *MyHardCutEffect_shader=
"vec4 FUNCNAME( vec2 tc ) {\n"
"	if ( PREFIX(show_second) > 0.0 )\n"
"		return INPUT2( tc );\n"
"	return INPUT1( tc );\n"
"}\n";



class MyHardCutEffect : public Effect {
public:
	MyHardCutEffect() : show_second( 0 ) {
		register_float( "show_second", &show_second );
	}	
	virtual std::string effect_type_id() const { return "MyHardCutEffect"; }
	std::string output_fragment_shader() { return MyHardCutEffect_shader; }
	virtual bool needs_srgb_primaries() const { return false; }
	virtual unsigned num_inputs() const { return 2; }

private:
	float show_second;
};



class GLHardCut : public GLFilter
{
public:
	GLHardCut( QString id, QString name );

	bool process( const QList<Effect*>&, Frame *src, Frame *dst, Profile *p );
	QList<Effect*> getMovitEffects();

private:
	Parameter *position;
};

#endif //GLHARDCUT_H
