#include <vector>

#include "engine/util.h"
#include "vfx/gldropshadow.h"



GLDropShadow::GLDropShadow( QString id, QString name ) : GLFilter( id, name )
{
	color = addParameter( "color", tr("Shadow color:"), Parameter::PRGBCOLOR, QColor::fromRgbF( 0, 0, 0 ), QColor::fromRgbF( 0, 0, 0 ), QColor::fromRgbF( 1, 1, 1 ), false );
	xoffset = addParameter( "xoffset", tr("X offset:"), Parameter::PDOUBLE, 10.0, -100.0, 100.0, true );
	yoffset = addParameter( "yoffset", tr("Y offset:"), Parameter::PDOUBLE, 10.0, -100.0, 100.0, true );
	opacity = addParameter( "opacity", tr("Opacity:"), Parameter::PDOUBLE, 0.8, 0.0, 1.0, true );
	radius = addParameter( "radius", tr("Blur radius:"), Parameter::PDOUBLE, 4.0, 0.0, 50.0, true );
}



GLDropShadow::~GLDropShadow()
{
}



bool GLDropShadow::process( const QList<Effect*> &el, Frame *src, Profile *p )
{
	Q_UNUSED( p );
	double pts = src->pts();
	QColor c = getParamValue( color ).value<QColor>();
	// convert gamma and premultiply
	sRgbColorToLinear( c );
	RGBTriplet col = RGBTriplet( c.redF(), c.greenF(), c.blueF() );
	Effect *e = el[0];
	return e->set_float( "xoffset", getParamValue( xoffset, pts ).toFloat() )
		&& e->set_float( "yoffset", getParamValue( yoffset, pts ).toFloat() )
		&& e->set_float( "opacity", getParamValue( opacity, pts ).toFloat() )
		&& e->set_float( "radius", getParamValue( radius, pts ).toFloat() )
		&& e->set_vec3( "color", (float*)&col );
}



QList<Effect*> GLDropShadow::getMovitEffects()
{
	QList<Effect*> list;
	list.append( new MyDropShadowEffect() );
	return list;
}



MyDropShadowEffect::MyDropShadowEffect()
	: blur(new BlurEffect),
	  shadow(new MyShadowMapEffect),
	  overlay(new OverlayEffect)
{
}



void MyDropShadowEffect::rewrite_graph(EffectChain *graph, Node *self)
{
	assert(self->incoming_links.size() == 1);
	Node *input = self->incoming_links[0];

	Node *shadow_node = graph->add_node(shadow);
	Node *blur_node = graph->add_node(blur);
	Node *overlay_node = graph->add_node(overlay);
	graph->replace_receiver(self, shadow_node);
	graph->connect_nodes(shadow_node, blur_node);
	graph->connect_nodes(blur_node, overlay_node);
	graph->connect_nodes(input, overlay_node);
	graph->replace_sender(self, overlay_node);

	self->disabled = true;
}



bool MyDropShadowEffect::set_float(const std::string &key, float value)
{
	if (key == "radius")
		return blur->set_float("radius", value);
	if (key == "xoffset")
		return shadow->set_float("xoffset", value);
	if (key == "yoffset")
		return shadow->set_float("yoffset", value);
	
	return shadow->set_float("opacity", value);
}



bool MyDropShadowEffect::set_vec3(const std::string &key, const float *values)
{
	if (key == "color")
		return shadow->set_vec3("color", values);
	
	return false;
}



MyShadowMapEffect::MyShadowMapEffect()
	: xoffset(10.0f),
	yoffset(10.0f),
	opacity(0.9f),
	color(0.0f,0.0f,0.0f)
{
	register_float("xoffset", &xoffset);
	register_float("yoffset", &yoffset);
	register_float("opacity", &opacity);
	register_vec3("color", (float*)&color);
}
