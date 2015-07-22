#include "glfrostedglass.h"



GLFrostedGlass::GLFrostedGlass( QString id, QString name ) : GLFilter( id, name )
{
	position = addParameter( "position", tr("Position:"), Parameter::PDOUBLE, 0.0, 0.0, 1.0, true );
	position->graph.keys.append( AnimationKey( AnimationKey::CURVE, 0, 0 ) );
	position->graph.keys.append( AnimationKey( AnimationKey::CURVE, 0.5, 0.5 ) );
	position->graph.keys.append( AnimationKey( AnimationKey::CURVE, 1, 0 ) );
	position->hidden = true;
	
	mixAmount = addParameter( "mixAmount", tr("Amount:"), Parameter::PDOUBLE, 0.0, 0.0, 1.0, true );
	mixAmount->graph.keys.append( AnimationKey( AnimationKey::CURVE, 0, 1 ) );
	mixAmount->graph.keys.append( AnimationKey( AnimationKey::CURVE, 1, 0 ) );
	mixAmount->hidden = true;
}



bool GLFrostedGlass::process( const QList<Effect*> &el, double pts, Frame *first, Frame *second, Profile *p )
{
	Q_UNUSED( first );
	Q_UNUSED( second );
	Q_UNUSED( p );
	Effect *e = el[0];
	return e->set_float( "radius", first->glWidth * 0.05 )
		&& e->set_float( "cover_position", getParamValue( position, pts ).toDouble() )
		&& e->set_float( "mix", getParamValue( mixAmount, pts ).toDouble() );
}



QList<Effect*> GLFrostedGlass::getMovitEffects()
{
	QList<Effect*> list;
	list.append( new MyFrostedGlassEffect() );
	return list;
}



MyFrostedGlassEffect::MyFrostedGlassEffect()
	: blur1( new BlurEffect ),
	  blur2( new BlurEffect ),
	  cover1( new MySlidingWindow ),
	  cover2( new MySlidingWindow ),
	  strength_first( 1 ),
	  strength_second( 0 ),
	  position( 0 )
{
	register_float("strength_first", &strength_first);
	register_float("strength_second", &strength_second);
	register_float( "position", &position );
}



bool MyFrostedGlassEffect::set_float( const std::string &key, float value )
{
	if ( key == "radius" )
		return blur1->set_float( "radius", value ) && blur2->set_float( "radius", value );
	if ( key == "mix" )
		return Effect::set_float( "strength_first", value ) && Effect::set_float( "strength_second", 1.0f - value );
	if ( key == "cover_position" )
		return cover1->set_float( "position", value ) && cover2->set_float( "position", value ) && Effect::set_float( "position", value );
	
	return false;
}



void MyFrostedGlassEffect::rewrite_graph( EffectChain *graph, Node *self )
{
	assert(self->incoming_links.size() == 2);
	Node *input1 = self->incoming_links[0];
	Node *input2 = self->incoming_links[1];
	self->incoming_links.clear();
	
	Node *blur1_node = graph->add_node( blur1 );
	Node *blur2_node = graph->add_node( blur2 );
	Node *cover1_node = graph->add_node( cover1 );
	Node *cover2_node = graph->add_node( cover2 );
	
	blur1_node->incoming_links.push_back( input1 );
	for ( unsigned j = 0; j < input1->outgoing_links.size(); ++j ) {
		if ( input1->outgoing_links[j] == self ) {
			input1->outgoing_links[j] = blur1_node;
		}
	}
	graph->connect_nodes( input1, cover1_node );
	graph->connect_nodes( blur1_node, cover1_node );
	graph->connect_nodes( cover1_node, self );
	
	blur2_node->incoming_links.push_back( input2 );
	for ( unsigned j = 0; j < input2->outgoing_links.size(); ++j ) {
		if ( input2->outgoing_links[j] == self ) {
			input2->outgoing_links[j] = blur2_node;
		}
	}
	graph->connect_nodes( input2, cover2_node );
	graph->connect_nodes( blur2_node, cover2_node );
	graph->connect_nodes( cover2_node, self );
}
