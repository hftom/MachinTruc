#ifndef GLMASK_H
#define GLMASK_H

#include "vfx/glfilter.h"
#include "engine/util.h"



class MaskEffect : public Effect
{
public:
	MaskEffect();
	std::string effect_type_id() const;
	std::string output_fragment_shader();

private:
	RGBTriplet hsvColor;
	float varianceH, varianceS, varianceV, smoothSelect;
};



class MixMaskEffect : public Effect
{
public:
	MixMaskEffect();
	std::string effect_type_id() const;
	std::string output_fragment_shader();
	unsigned num_inputs() const { return 3; }

private:
	int invert, show;
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
	Parameter *selectionMode;
	Parameter *hsvColor, *varianceH, *varianceS, *varianceV, *invertColor, *showColor, *smoothSelect;
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

	bool set_int(const std::string &key, int value) {
		return realEffect->set_int(key, value);
	}
	bool set_float(const std::string &key, float value) {
		return realEffect->set_float(key, value);
	}
	bool set_vec2(const std::string &key, const float *values) {
		return realEffect->set_vec2(key, values);
	}
	bool set_vec3(const std::string &key, const float *values) {
		return realEffect->set_vec3(key, values);
	}
	bool set_vec4(const std::string &key, const float *values) {
		return realEffect->set_vec4(key, values);
	}


protected:
	Effect *realEffect;
	GLMask *glMask;
};

#endif // GLMASK_H
