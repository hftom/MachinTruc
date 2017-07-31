#ifndef GLMASK_H
#define GLMASK_H

#include "vfx/glfilter.h"
#include "engine/util.h"



static const char* mask_shader =
"vec3 PREFIX(rgb2hsv)(vec3 rgb) {\n"
"	vec3 hsv;\n"
"	float min = rgb.r < rgb.g ? rgb.r : rgb.g;\n"
"	min = min < rgb.b ? min : rgb.b;\n"
"	float max = rgb.r > rgb.g ? rgb.r : rgb.g;\n"
"	max = max > rgb.b ? max : rgb.b;\n"
"	hsv.z = max;\n"
"	float delta = max - min;\n"
"	if (delta < 0.00001 || max <= 0.0) {\n"
"		hsv.y = 0.0;\n"
"		hsv.x = PREFIX(hsvColor).x;\n"
"		return hsv;\n"
"	}\n"
"	hsv.y = (delta / max);\n"
"	if ( rgb.r == max )\n"
"		hsv.x = ( rgb.g - rgb.b ) / delta;\n"
"	else if ( rgb.g >= max )\n"
"		hsv.x = 2.0 + ( rgb.b - rgb.r ) / delta;\n"
"	else\n"
"		hsv.x = 4.0 + ( rgb.r - rgb.g ) / delta;\n"
"\n"
"	hsv.x *= 60.0;\n"
"	if ( hsv.x < 0.0 )\n"
"		hsv.x += 360.0;\n"
"\n"
"	return hsv;\n"
"}\n"
"\n"
"vec4 FUNCNAME(vec2 tc) {\n"
"	vec3 hsv = PREFIX(rgb2hsv)(INPUT(tc).rgb);\n"
"	if ( (distance(hsv.x, PREFIX(hsvColor).x) < PREFIX(varianceH)\n"
"		|| distance(hsv.x - 360.0, PREFIX(hsvColor).x) < PREFIX(varianceH)\n"
"		|| distance(hsv.x + 360.0, PREFIX(hsvColor).x) < PREFIX(varianceH))\n"
"		&& distance(hsv.y, PREFIX(hsvColor).y) < PREFIX(varianceS)\n"
"		&& distance(hsv.z, PREFIX(hsvColor).z) < PREFIX(varianceV) )\n"
"	{\n"
"		return vec4(1);\n"
"	}\n"
"\n"
"	return vec4(0);\n"
"}\n";



class MaskEffect : public Effect
{
public:
	MaskEffect() : hsvColor(0.0f,0.0f,0.5f), varianceH(0), varianceS(0), varianceV(0) {
		register_vec3("hsvColor", (float*)&hsvColor);
		register_float("varianceH", &varianceH);
		register_float("varianceS", &varianceS);
		register_float("varianceV", &varianceV);
	}
	std::string effect_type_id() const {return "MaskEffect";}
	std::string output_fragment_shader() {return mask_shader;}

private:
	RGBTriplet hsvColor;
	float varianceH, varianceS, varianceV;
};



static const char* mix_mask_shader =
"vec4 FUNCNAME(vec2 tc) {\n"
"	vec4 org = INPUT1(tc);\n"
"	vec4 effect = INPUT2(tc);\n"
"	float mask = INPUT3(tc).a;\n"
"	if (PREFIX(invert) > 0)\n"
"		mask = 1.0 - mask;\n"
"\n"
"	return vec4(mix( org, effect, mask ));\n"
"}\n";



class MixMaskEffect : public Effect
{
public:
	MixMaskEffect() : invert(0) {
		register_float("invert", &invert);
	}
	std::string effect_type_id() const {return "MixMaskEffect";}
	std::string output_fragment_shader() {return mix_mask_shader;}
	unsigned num_inputs() const { return 3; }

private:
	float invert;
};



class GLMask : public GLFilter
{
public:
	GLMask( QString id, QString name );

	void setParameters();
	void setGraph(EffectChain *graph, Node *input, Node *receiverSender, Node *effect);

	QString getMaskDescriptor( double pts, Frame *src, Profile *p  );
	bool processMask( double pts, Frame *src, Profile *p );

private:
	Effect *mask, *mix;
	Parameter *selectionMode, *hsvColor, *varianceH, *varianceS, *varianceV, *invert;
};



class PseudoEffect : public Effect
{
public:
	PseudoEffect(GLMask *parent, Effect *effect) : realEffect(effect), glMask(parent) {}
	std::string effect_type_id() const {return "PseudoEffect";}
	std::string output_fragment_shader() {assert(false);}

	void rewrite_graph(EffectChain *graph, Node *self) {
		Node *input = self->incoming_links[0];
		Node *real_node = graph->add_node(realEffect);
		glMask->setGraph(graph, input, self, real_node);

		self->disabled = true;
	}

	bool set_float(const std::string &key, float value) {
		realEffect->set_float(key, value);
	}
	bool set_vec2(const std::string &key, const float *values) {
		realEffect->set_vec2(key, values);
	}
	bool set_vec3(const std::string &key, const float *values) {
		realEffect->set_vec3(key, values);
	}
	bool set_vec4(const std::string &key, const float *values) {
		realEffect->set_vec4(key, values);
	}


protected:
	Effect *realEffect;
	GLMask *glMask;
};

#endif // GLMASK_H
