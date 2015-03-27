#ifndef MOVITCHAIN_H
#define MOVITCHAIN_H

#define GL_GLEXT_PROTOTYPES

#include <movit/effect_chain.h>
#include <movit/resource_pool.h>
#include <movit/effect.h>
#include <movit/input.h>

#include "engine/frame.h"
#include "vfx/glfilter.h"

// Do not define unless you know what you do!
//#define NOMOVIT

using namespace movit;



static const char *BlankInput_shader=
"vec4 FUNCNAME(vec2 tc) {\n"
"	return vec4( 0.0 );\n"
"}\n";



class BlankInput : public Input
{
public:
	BlankInput( int w, int h ) : width( w ), height( h ), output_linear_gamma(true), needs_mipmaps(false) {
		register_int("output_linear_gamma", &output_linear_gamma);
		register_int("needs_mipmaps", &needs_mipmaps);
	}
	std::string effect_type_id() const { return "BlankInput"; }
	std::string output_fragment_shader() { return BlankInput_shader; }
	AlphaHandling alpha_handling() const { return INPUT_AND_OUTPUT_PREMULTIPLIED_ALPHA; }
	bool can_output_linear_gamma() const { return true; }
	unsigned get_width() const { return width; }
	unsigned get_height() const { return height; }
	Colorspace get_color_space() const { return COLORSPACE_sRGB; }
	GammaCurve get_gamma_curve() const { return GAMMA_LINEAR; }
	
private:
	int width, height;
	int output_linear_gamma, needs_mipmaps;
};



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
	QString mmiProvider;
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
	
	QStringList descriptor;
};

#endif //MOVITCHAIN_H
