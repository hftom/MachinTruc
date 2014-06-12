#include <assert.h>
#include <vector>

#include <movit/effect_chain.h>

#include "vfx/glsaturation.h"



GLSaturation::GLSaturation( QString id, QString name ) : GLFilter( id, name )
{
	saturation = 2.0;
	addParameter( tr("Saturation:"), PFLOAT, 0.0, 5.0, true, &saturation );
}



GLSaturation::~GLSaturation()
{
}



bool GLSaturation::process( Effect *e, Frame *src, Profile *p )
{
	Q_UNUSED( src );
	Q_UNUSED( p );
	return e->set_float( "saturation", saturation );
}



Effect* GLSaturation::getMovitEffect()
{
	return new MGLSaturation();
}



MGLSaturation::MGLSaturation()
	: sat(new SaturationEffect),
	  maskfx(new SMaskEffect)
{
}

void MGLSaturation::rewrite_graph(EffectChain *graph, Node *self)
{
	assert(self->incoming_links.size() == 1);
	Node *input = self->incoming_links[0];

	Node *sat_node = graph->add_node(sat);
	Node *mask_node = graph->add_node(maskfx);
	graph->replace_receiver(self, mask_node);
	graph->connect_nodes(input, sat_node);
	graph->connect_nodes(sat_node, mask_node);
	graph->replace_sender(self, mask_node);

	self->disabled = true;
}

bool MGLSaturation::set_float(const std::string &key, float value)
{
	return sat->set_float(key, value);
}
