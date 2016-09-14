#ifndef GLSTABILIZE_H
#define GLSTABILIZE_H

extern "C" { 
	#include <vid.stab/libvidstab.h>
}

#include <movit/effect_util.h>
#include <movit/effect_chain.h>
#include <movit/util.h>

#include <QTimer>
#include <QMutexLocker>

#include "engine/stabilizecollection.h"

#include "glfilter.h"



static const char *MyStabilizeEffect_shader=
"uniform vec2 PREFIX(factor);\n"
"uniform vec2 PREFIX(cosSin);\n"
"uniform vec2 PREFIX(offset);\n"
"\n"
"vec4 FUNCNAME( vec2 tc ) {\n"
"	tc -= 0.5;\n"
"	tc *= PREFIX(factor);\n"
"	tc *= mat2( PREFIX(cosSin).x, PREFIX(cosSin).y, -PREFIX(cosSin).y, PREFIX(cosSin).x );\n"
"	tc /= PREFIX(factor);\n"
"	tc -= PREFIX(offset);\n"
"	tc += 0.5;\n"
"	return clamp( INPUT( tc ), vec4(0.0), vec4(1.0) );\n"
"}\n";



class MyStabilizeEffect : public Effect {
public:
	MyStabilizeEffect() : iwidth(1), iheight(1), top(0), left(0), angle(0.0), SAR(1.0), zoom(1.0) {
		register_float("angle", &angle);
		register_float("SAR", &SAR);
		register_float("top", &top);
		register_float("left", &left);
		register_float("zoom", &zoom);
	}
	
	virtual std::string effect_type_id() const { return "MyStabilizeEffect"; }
	std::string output_fragment_shader() { return MyStabilizeEffect_shader; }
	
	virtual void inform_input_size(unsigned, unsigned width, unsigned height) {
		iwidth = width;
		iheight = height;
	}
	
	virtual void set_gl_state( GLuint glsl_program_num, const std::string &prefix, unsigned *sampler_num ) {
		Q_UNUSED( sampler_num );
		float offset[2] = { left / iwidth, - top / iheight };
		set_uniform_vec2( glsl_program_num, prefix, "offset", offset );
	
		// we have to take both texture_size_ratio and sample_aspect_ratio into account.
		float factor[2] = { SAR * iwidth / iheight, 1.0f };
		set_uniform_vec2( glsl_program_num, prefix, "factor", factor );
		
		float cosSin[2] = { zoom * (float)cos( angle ),  zoom * (float)sin( angle ) };
		set_uniform_vec2( glsl_program_num, prefix, "cosSin", cosSin );
	}

private:
	float iwidth, iheight;
	float top, left;
	float angle;
	float SAR;
	float zoom;
};



class GLStabilize : public GLFilter
{
	Q_OBJECT
public:
	GLStabilize( QString id, QString name );
	~GLStabilize();
 
	virtual bool process( const QList<Effect*> &el, double pts, Frame *src, Profile *p );
	QList<Effect*> getMovitEffects();
	
	void setSource( Source *aSource );
	void filterRemoved();

private slots:
	void checkStabData();
		
private:
	Source *source;
	QList<StabilizeTransform> *transforms;
	QTimer checkStabTimer;
	Parameter *stabStatus, *strength;
	
	QMutex transformsMutex;
	
signals:
	void statusUpdate( QString );
};

#endif //GLSTABILIZE_H
