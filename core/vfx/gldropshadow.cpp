#include <vector>

#include "vfx/gldropshadow.h"



GLDropShadow::GLDropShadow( QString id, QString name ) : GLFilter( id, name )
{
	opacity = 0.9;
	xoffset = yoffset = 10.0;
	radius = 4.0;
	addParameter( tr("X offset:"), PFLOAT, -100.0, 100.0, true, &xoffset );
	addParameter( tr("Y offset:"), PFLOAT, -100.0, 100.0, true, &yoffset );
	addParameter( tr("Opacity:"), PFLOAT, 0.0, 1.0, true, &opacity );
	addParameter( tr("Blur radius:"), PFLOAT, 0.0, 50.0, true, &radius );
}



GLDropShadow::~GLDropShadow()
{
}



bool GLDropShadow::process( const QList<Effect*> &el, Frame *src, Profile *p )
{
	Q_UNUSED( src );
	Q_UNUSED( p );
	return el.at(0)->set_float( "xoffset", xoffset )
		&& el.at(0)->set_float( "yoffset", yoffset )
		&& el.at(0)->set_float( "opacity", opacity )
		&& el.at(0)->set_float( "radius", radius );
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



bool MyDropShadowEffect::set_float(const std::string &key, float value) {
	if (key == "radius")
		return blur->set_float("radius", value);
	if (key == "xoffset")
		return shadow->set_float("xoffset", value);
	if (key == "yoffset")
		return shadow->set_float("yoffset", value);
	
	return shadow->set_float("opacity", value);
}



MyShadowMapEffect::MyShadowMapEffect()
	: xoffset(10.0),
	yoffset(10.0),
	opacity(0.9)
{
	register_float("xoffset", &xoffset);
	register_float("yoffset", &yoffset);
	register_float("opacity", &opacity);
}
