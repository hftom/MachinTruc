#include "gldenoise.h"



GLDenoise::GLDenoise( QString id, QString name ) : GLFilter( id, name )
{
	blur = addParameter( "blur", tr("Denoise blur:"), Parameter::PDOUBLE, 0.12, 0.0, 2.0, false, "%" );
	amp = addParameter( "amp", tr("Edge strength:"), Parameter::PDOUBLE, 10.0, 0.0, 20.0, false );
	eblur = addParameter( "eblur", tr("Edge smoothing:"), Parameter::PDOUBLE, 1.0, 0.0, 4.0, false );
}



bool GLDenoise::process( const QList<Effect*> &el, double pts, Frame *src, Profile *p )
{
	Q_UNUSED( p );
	Effect *e = el[0];
	return e->set_float( "radius", src->glWidth * getParamValue( blur ).toFloat() / 100.0 )
		&& e->set_float( "eradius", getParamValue( eblur ).toFloat() )
		&& e->set_float( "amp", getParamValue( amp ).toFloat() );
}



QList<Effect*> GLDenoise::getMovitEffects()
{
	QList<Effect*> list;
	list.append( new MyDenoiseEffect() );
	return list;
}



MyDenoiseEffect::MyDenoiseEffect()
	: blur( new BlurEffect ),
	  eblur( new BlurEffect ),
	  edge( new DenoiseEdgeEffect ),
	  mask( new MyDenoiseMask )
{
	bool ok = eblur->set_int( "num_taps", 6 );
	ok |= blur->set_int( "num_taps", 8 );
}



bool MyDenoiseEffect::set_float( const std::string &key, float value )
{
	if ( key == "radius" )
		return blur->set_float( "radius", value );
	if ( key == "eradius" )
		return eblur->set_float( "radius", value );
	if ( key == "amp" )
		return edge->set_float( "amp", value );
	
	return false;
}



void MyDenoiseEffect::rewrite_graph( EffectChain *graph, Node *self )
{
	assert(self->incoming_links.size() == 1);
	Node *input = self->incoming_links[0];

	Node *blur_node = graph->add_node(blur);
	Node *eblur_node = graph->add_node(eblur);
	Node *edge_node = graph->add_node(edge);
	Node *mask_node = graph->add_node(mask);
	graph->replace_receiver(self, eblur_node);
	graph->connect_nodes(eblur_node, edge_node);
	graph->connect_nodes(edge_node, mask_node);
	graph->connect_nodes(input, blur_node);
	graph->connect_nodes(blur_node, mask_node);
	graph->connect_nodes(input, mask_node);
	graph->replace_sender(self, mask_node);
	
	self->disabled = true;
}
