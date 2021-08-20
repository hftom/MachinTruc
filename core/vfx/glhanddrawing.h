#ifndef GLHANDDRAWING_H
#define GLHANDDRAWING_H

#include <movit/effect.h>
#include <movit/effect_util.h>

#include "vfx/glfilter.h"



class HandDrawingEdgeEffect : public Effect {
public:
	HandDrawingEdgeEffect() : iwidth(1), iheight(1) {
	}
	std::string effect_type_id() const { return "HandDrawingEdgeEffect"; }
	std::string output_fragment_shader() { return GLFilter::getShader("hand_drawing_edge.frag"); }
	bool needs_texture_bounce() const { return true; }
	
	virtual void inform_input_size(unsigned, unsigned width, unsigned height) {
		iwidth = width;
		iheight = height;
	}

	virtual void set_gl_state( GLuint glsl_program_num, const std::string &prefix, unsigned *sampler_num ) {
		Effect::set_gl_state( glsl_program_num, prefix, sampler_num );
		float size[2] = { 1.0f / iwidth, 1.0f / iheight };
		set_uniform_vec2( glsl_program_num, prefix, "pixelSize", size );
	}
	
private:
	float iwidth, iheight;
};



class HandDrawingDrawEffect : public Effect {
public:
	HandDrawingDrawEffect() {}
	virtual std::string effect_type_id() const { return "HandDrawingDrawEffect"; }
	std::string output_fragment_shader() { return GLFilter::getShader("hand_drawing.frag"); }
	virtual unsigned num_inputs() const { return 2; }	
	bool needs_texture_bounce() const { return true; }
};



class HandDrawingEffect : public Effect {
public:
	HandDrawingEffect() : edge( new HandDrawingEdgeEffect ), draw( new HandDrawingDrawEffect ) {}
	std::string effect_type_id() const { return "HandDrawingEffect"; }
	std::string output_fragment_shader() { assert(false); }
	void rewrite_graph(EffectChain *graph, Node *self) {
		assert(self->incoming_links.size() == 1);
		Node *input = self->incoming_links[0];

		Node *edge_node = graph->add_node(edge);
		Node *draw_node = graph->add_node(draw);
		graph->replace_receiver(self, edge_node);
		graph->connect_nodes(input, draw_node);
		graph->connect_nodes(edge_node, draw_node);
		graph->replace_sender(self, draw_node);
	
		self->disabled = true;
	}

private:
	HandDrawingEdgeEffect *edge;
	HandDrawingDrawEffect *draw;
};



class GLHandDrawing : public GLFilter
{
	Q_OBJECT
public:
	GLHandDrawing( QString id, QString name );
	~GLHandDrawing();

	bool process( const QList<Effect*> &el, double pts, Frame *src, Profile *p );

	QList<Effect*> getMovitEffects();
	
private:
	Parameter *size;
};

#endif //GLHANDDRAWING_H
