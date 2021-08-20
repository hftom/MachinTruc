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


class GLSLInput : public Input
{
public:
	GLSLInput( int w, int h, QString shader ) : iwidth(1), iheight(1), width( w ), height( h ), shaderName(shader), time(0), output_linear_gamma(true), needs_mipmaps(false) {
		register_float("iwidth", &iwidth);
		register_float("iheight", &iheight);
		register_float("time", &time);
		register_int("output_linear_gamma", &output_linear_gamma);
		register_int("needs_mipmaps", &needs_mipmaps);
		if (shaderName.isEmpty()) {
			shaderName = "Blank";
		}
	}
	std::string effect_type_id() const { return shaderName.toStdString(); }
	std::string output_fragment_shader() {
		QString s;
		if (shaderName == "OpticalFiber") s = "optical_fiber";
		else if (shaderName == "LaserGrid") s = "laser_grid";
		else if (shaderName == "Warp") s = "warp";
		else if (shaderName == "Warp2") s = "warp2";
		else if (shaderName == "Clouds") s = "clouds";
		else s = "blank";
		return GLFilter::getShader(s + "_input.frag");
	}
	AlphaHandling alpha_handling() const { return INPUT_AND_OUTPUT_PREMULTIPLIED_ALPHA; }
	bool can_output_linear_gamma() const { return true; }
	unsigned get_width() const { return width; }
	unsigned get_height() const { return height; }
	Colorspace get_color_space() const { return COLORSPACE_sRGB; }
	GammaCurve get_gamma_curve() const { return GAMMA_LINEAR; }

private:
	float iwidth, iheight;
	int width, height;
	QString shaderName;
	float time;
	int output_linear_gamma, needs_mipmaps;
};



class MovitInput
{
public:
	MovitInput();
	~MovitInput();

	bool process( Frame *src, double pts, GLResource *gl = NULL );
	Input* getMovitInput( Frame *src );

	static QString getDescriptor( Frame *src );

private:
	bool setBuffer( PBO *p, Frame *src, int size );
	void setPixelData8(Frame *src, int size, int stride[], GLResource *gl );
	void setPixelData16(Frame *src, int size,int stride[],  GLResource *gl );
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
