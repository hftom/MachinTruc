#ifndef GLSIZE_H
#define GLSIZE_H

#include <movit/effect_util.h>
#include <movit/effect_chain.h>
#include <movit/util.h>
#include <movit/resize_effect.h>
#include <movit/blur_effect.h>
#include <movit/padding_effect.h>
#include <movit/overlay_effect.h>

#include "glsoftborder.h"



class MyResizeOpaqueEffect : public Effect {
public:
	MyResizeOpaqueEffect() 	: width(1280), height(720)
	{
		register_int("width", &width);
		register_int("height", &height);
	}
	std::string effect_type_id() const override { return "MyResizeOpaqueEffect"; }
	std::string output_fragment_shader() override { return GLFilter::getShader("resize_opaque.frag"); }
	bool needs_texture_bounce() const override { return true; }
	bool changes_output_size() const override { return true; }
	bool sets_virtual_output_size() const override { return false; }
	void get_output_size(unsigned *width, unsigned *height, unsigned *virtual_width, unsigned *virtual_height) const override
	{
		*virtual_width = *width = this->width;
		*virtual_height = *height = this->height;
	}

private:
	int width, height;
};



class MyBlurFillerEffect : public Effect {
public:
	MyBlurFillerEffect() : blur(new BlurEffect), resize(new MyResizeOpaqueEffect), padding(new PaddingEffect), overlay(new OverlayEffect) {
	}
	std::string effect_type_id() const { return "MyBlurFillerEffect"; }
	std::string output_fragment_shader() { assert(false); }

	void rewrite_graph(EffectChain *graph, Node *self) {
		assert(self->incoming_links.size() == 1);
		Node *input = self->incoming_links[0];

		Node *resize_node = graph->add_node(resize);
		Node *blur_node = graph->add_node(blur);
		Node *padding_node = graph->add_node(padding);
		Node *overlay_node = graph->add_node(overlay);

		graph->replace_receiver(self, resize_node);
		graph->connect_nodes(resize_node, blur_node);
		graph->connect_nodes(input, padding_node);
		graph->connect_nodes(blur_node, overlay_node);
		graph->connect_nodes(padding_node, overlay_node);
		graph->replace_sender(self, overlay_node);

		self->disabled = true;
	}

	bool setPadding(int ow, int oh, float top, float left) {
		bool ok = resize->set_int("width", ow);
		ok = resize->set_int("height", oh);
		ok = padding->set_int( "width", ow );
		ok = padding->set_int( "height", oh );
		ok = padding->set_float( "top", top );
		ok = padding->set_float( "left", left );
		ok = blur->set_float("radius", ow * 0.05f);
		return ok;
	}

private:
	BlurEffect *blur;
	MyResizeOpaqueEffect *resize;
	PaddingEffect *padding;
	OverlayEffect *overlay;
};



class MyRotateEffect : public Effect {
public:
	MyRotateEffect() : owidth(1), oheight(1), iwidth(1), iheight(1), top(0), left(0), centerOffsetX(0), centerOffsetY(0), angle(0.0), SAR(1.0) {
		register_float("angle", &angle);
		register_float("SAR", &SAR);
		register_int("width", &owidth);
		register_int("height", &oheight);
		register_float("top", &top);
		register_float("left", &left);
		register_float("centerOffsetX", &centerOffsetX);
		register_float("centerOffsetY", &centerOffsetY);
	}
	
	virtual std::string effect_type_id() const { return "MyRotateEffect"; }
	std::string output_fragment_shader() { return GLFilter::getShader("rotate.frag"); }
	
	virtual void inform_input_size(unsigned, unsigned width, unsigned height) {
		iwidth = width;
		iheight = height;
	}
	
	virtual bool changes_output_size() const { return true; }
	
	virtual void get_output_size(unsigned *width, unsigned *height, unsigned *virtual_width, unsigned *virtual_height) const {
		*virtual_width = *width = owidth;
		*virtual_height = *height = oheight;
	}
	
	virtual void set_gl_state( GLuint glsl_program_num, const std::string &prefix, unsigned *sampler_num ) {
		//Effect::set_gl_state( glsl_program_num, prefix, sampler_num );
		Q_UNUSED( sampler_num );
		float offset[2] = { left / owidth, ( oheight - iheight - top ) / oheight };
		set_uniform_vec2( glsl_program_num, prefix, "offset", offset );

		float scale[2] = { float(owidth) / iwidth, float(oheight) / iheight };
		set_uniform_vec2( glsl_program_num, prefix, "scale", scale );
		
		float centerOffset[2] = { 0.5f + (centerOffsetX / iwidth), 0.5f + (centerOffsetY / iheight) };
		set_uniform_vec2( glsl_program_num, prefix, "centerOffset", centerOffset );
	
		// we have to take both texture_size_ratio and sample_aspect_ratio into account.
		float factor[2] = { SAR * iwidth / iheight, 1.0f };
		set_uniform_vec2( glsl_program_num, prefix, "factor", factor );
		
		float cosSin[2] = { (float)cos( angle ),  (float)sin( angle ) };
		set_uniform_vec2( glsl_program_num, prefix, "cosSin", cosSin );
	}

private:
	int owidth, oheight;
	float iwidth, iheight;
	float top, left;
	float centerOffsetX, centerOffsetY;
	float angle;
	float SAR;
};



class GLSize : public GLFilter
{
	Q_OBJECT
public:
	GLSize( QString id, QString name );
	~GLSize();
 
	virtual QString getDescriptor( double pts, Frame *src, Profile *p );
	virtual bool process( const QList<Effect*> &el, double pts, Frame *src, Profile *p );
	virtual void ovdUpdate( QString type, QVariant val );

	QList<Effect*> getMovitEffects();

		
protected:
	virtual void preProcess(double pts, Frame *src, Profile *p ) {
		Q_UNUSED(pts); Q_UNUSED(src); Q_UNUSED(p);
	}
	void findPoints( double &x1, double &x2, double first, double last );
	
	Parameter *sizePercent;
	bool resizeActive;
	int resizeOutputWidth, resizeOutputHeight;
	double resizeZoomX, resizeZoomY;
	double resizeLeft, resizeTop;
	double resizeZoomCenterX, resizeZoomCenterY;
	
	Parameter *rotateAngle;//, *softBorder;
	bool rotateActive;
	double centerOffsetX, centerOffsetY;
	
	Parameter *xOffset, *yOffset;
	QVariant ovdOffset, ovdScale;

	Parameter *blurFiller;
	bool blurFillerActive;
	bool softBorderActive;
};

#endif //GLSIZE_H
