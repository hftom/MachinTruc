#ifndef GLHANDDRAWING_H
#define GLHANDDRAWING_H

#include <movit/effect.h>
#include <movit/effect_util.h>

#include "vfx/glfilter.h"



/* based on work by florian berger (flockaroo) - 2016
* License Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License.
*/
static const char *HandDrawingEdge_shader=
"uniform vec2 PREFIX(pixelSize);\n"
"\n"
"float PREFIX(gray)(vec2 tc) {\n"
"	return dot(INPUT(tc).rgb, vec3(1.0 / 3.0));\n"
"}\n"
"\n"
"vec4 FUNCNAME(vec2 tc) {\n"
"	vec2 offset = vec2(1.0, 0.0);\n"
"	return vec4(\n"
"		PREFIX(gray)(tc + PREFIX(pixelSize) * offset.xy) - PREFIX(gray)(tc - PREFIX(pixelSize) * offset.xy),\n"
"		PREFIX(gray)(tc + PREFIX(pixelSize) * offset.yx) - PREFIX(gray)(tc - PREFIX(pixelSize) * offset.yx),\n"
"		0.0, 0.0\n"
"	) / 2.0;\n"
"}\n";



class HandDrawingEdgeEffect : public Effect {
public:
	HandDrawingEdgeEffect() : iwidth(1), iheight(1) {
	}
	std::string effect_type_id() const { return "HandDrawingEdgeEffect"; }
	std::string output_fragment_shader() { return HandDrawingEdge_shader; }
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



static const char *HandDrawingDrawEffect_shader=
"const float PREFIX(pi2) = 6.28318530717959;\n"
"\n"
"vec4 getCol(vec2 pos) {\n"
"	return INPUT1(pos / vec2(1920.0, 1080.0));\n"
"}\n"
"\n"
"vec4 getColHT(vec2 pos) {\n"
"	return getCol(pos);//smoothstep(.95,1.05,getCol(pos)*.8+.2+getRand(pos*.7));\n"
"}\n"
"\n"
"vec4 FUNCNAME(vec2 tc) {\n"
"	float SampNum = 12.0;\n"
"	float stretch = 2.2;\n"
"	vec2 iResolution = vec2(1920.0, 1080.0);\n"
"\n"	
"\n"	
"	vec2 pos = (tc * iResolution);\n"
"	vec3 col = vec3(0);\n"
"	vec3 col2 = vec3(0);\n"
"	float sum = 0.0;\n"
"	for (float i = 0.0; i < 3.0; i++) {\n"
"		float ang = PREFIX(pi2) / 3.0 * (i + 0.8);\n"
"   	vec2 v = vec2(cos(ang), sin(ang));\n"
"		for (float j = 0.0; j < SampNum; j++) {\n"
"			vec2 dpos = v.yx * vec2(1.0, -1.0) * j * stretch;\n"
"			vec2 dpos2 = v.xy * j * j / SampNum * 0.5 * stretch;\n"
"			vec2 g;\n"
"			float fact;\n"
"			float fact2;\n"
"			for (float s = -1.0; s <= 1.0; s += 2.0) {\n"
"				vec2 pos2 = pos + s * dpos + dpos2;\n"
"				vec2 pos3 = pos + (s * dpos + dpos2).yx * vec2(1.0, -1.0) * 2.0;\n"
"				g = INPUT2(pos2 / vec2(1920.0, 1080.0)).rg;\n"
"				fact = dot(g, v) - 0.5 * abs(dot(g, v.yx * vec2(1.0, -1.0)));\n"
"				fact2 = dot(normalize(g + vec2(0.0001)), v.yx * vec2(1.0, -1.0));\n"
"				fact = clamp(fact, 0.0, 0.05);\n"
"				fact2 = abs(fact2);\n"
"				fact *= 1.0 - j / SampNum;\n"
"				col += fact;\n"
"				col2 += fact2 * getColHT(pos3).xyz;\n"
"				sum += fact2;\n"
"			}\n"
"		}\n"
"	}\n"
"	col /= SampNum * 2.25 / sqrt(iResolution.y);\n"
"	col2 /= sum;\n"
"	//col.x *= (0.6 + 0.8 * getRand(pos * 0.7).x);\n"
"	col.x = 1.0 - col.x;\n"
"	col.x *= col.x * col.x;\n"
"\n"
"	return vec4(vec3(col.x * col2), 1.0);\n"
"}\n";



class HandDrawingDrawEffect : public Effect {
public:
	HandDrawingDrawEffect() {}
	virtual std::string effect_type_id() const { return "HandDrawingDrawEffect"; }
	std::string output_fragment_shader() { return HandDrawingDrawEffect_shader; }
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
public:
	GLHandDrawing( QString id, QString name );
	~GLHandDrawing();

	bool process( const QList<Effect*> &el, double pts, Frame *src, Profile *p );

	QList<Effect*> getMovitEffects();
	
private:
	Parameter *size;
};

#endif //GLHANDDRAWING_H
