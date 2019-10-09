#include <vector>

#include "engine/util.h"
#include "vfx/gldropshadow.h"



GLDropShadow::GLDropShadow( QString id, QString name ) : GLFilter( id, name )
{
	color = addParameter( "color", tr("Shadow color:"), Parameter::PRGBCOLOR, QColor::fromRgbF( 0, 0, 0 ), QColor::fromRgbF( 0, 0, 0 ), QColor::fromRgbF( 1, 1, 1 ), false );
	xOffset = addParameter( "xOffset", tr("X offset:"), Parameter::PINPUTDOUBLE, 0.5, -110.0, 110.0, true, "%" );
	yOffset = addParameter( "yOffset", tr("Y offset:"), Parameter::PINPUTDOUBLE, 0.5, -110.0, 110.0, true, "%" );
	opacity = addParameter( "opacity", tr("Opacity:"), Parameter::PDOUBLE, 0.8, 0.0, 1.0, true );
	blur = addParameter( "blur", tr("Blur:"), Parameter::PDOUBLE, 0.2, 0.0, 5.0, true, "%" );
}



GLDropShadow::~GLDropShadow()
{
}



void GLDropShadow::ovdUpdate( QString type, QVariant val )
{
	if ( type == "translate" ) {
		ovdOffset = val;
	}
}



bool GLDropShadow::process( const QList<Effect*> &el, double pts, Frame *src, Profile *p )
{
	Q_UNUSED( p );
	QColor c = getParamValue( color ).value<QColor>();
	// convert gamma and premultiply
	sRgbColorToLinear( c );
	RGBTriplet col = RGBTriplet( c.redF(), c.greenF(), c.blueF() );
	
	if (!ovdOffset.isNull()) {
		QPointF pos = ovdOffset.toPointF();
		if ( !xOffset->graph.keys.count() )
			xOffset->value = pos.x() * 100.0 / src->glWidth;
		if ( !yOffset->graph.keys.count() )
			yOffset->value = pos.y() * 100.0 / src->glHeight;
		
		ovdOffset = QVariant();
	}
	
	double xof = src->glWidth * getParamValue( xOffset, pts ).toDouble() / 100.0;
	double yof = src->glHeight * getParamValue( yOffset, pts ).toDouble() / 100.0;
	
	if ( ovdEnabled() ) {
		src->glOVD = FilterTransform::TRANSLATE;
		src->glOVDRect = QRectF( -(double)src->glWidth / 2.0, -(double)src->glHeight / 2.0, src->glWidth, src->glHeight );
		src->glOVDTransformList.append( FilterTransform( FilterTransform::TRANSLATE, xof, yof ) );
	}
	
	Effect *e = el[0];
	return e->set_float( "xoffset", xof )
	&& e->set_float( "yoffset", yof )
		&& e->set_float( "opacity", getParamValue( opacity, pts ).toFloat() )
		&& e->set_float( "radius", src->glWidth * getParamValue( blur, pts ).toDouble() / 100.0 )
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
	: iwidth( 1 ),
	iheight( 1 ),
	xoffset(10.0f),
	yoffset(10.0f),
	opacity(0.9f),
	color(0.0f,0.0f,0.0f)
{
	register_float("xoffset", &xoffset);
	register_float("yoffset", &yoffset);
	register_float("opacity", &opacity);
	register_vec3("color", (float*)&color);
}
