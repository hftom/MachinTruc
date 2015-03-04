#include <QDebug>

#include "filter.h"



Filter::Filter( QString id, QString name )
	: identifier( id ),
	filterName( name ),
	posInTrack( 0 ),
	length( 0 )
{
}



Filter::~Filter()
{
	while ( !parameters.isEmpty() )
		delete parameters.takeFirst();
}



Parameter* Filter::addParameter( QString id, QString name, int type, QVariant def, QVariant min, QVariant max, bool keyframeable, const QString &suffix )
{
	Parameter *param = new Parameter();
	param->id = id;
	param->name = name;
	param->type = type;
	param->defValue = def;
	param->min = min;
	param->max = max;
	param->value = def;
	param->keyframeable = keyframeable;
	param->suffix = suffix;
	parameters.append( param );
	
	return param;
}



double Filter::getNormalizedTime( double pts )
{
	return (pts - posInTrack) / length;
}



QVariant Filter::getParamValue( Parameter *p, double pts )
{
	if ( !p->graph.keys.count() ) {
		return  p->value;
	}
	
	double range = qAbs( -p->min.toDouble() + p->max.toDouble() );
	return (range * p->graph.valueAt( getNormalizedTime( pts ) )) + p->min.toDouble();
}



void Filter::splitParameters( Filter *second, double posPts )
{
	QList<Parameter*> p_second = second->getParameters();
	QList<Parameter*> params = getParameters();
	for ( int i = 0; i < params.count(); ++i ) {
		p_second[i]->value = params[i]->value;
		AnimationGraph graph = params[i]->graph;
		params[i]->graph.keys.clear();
		if ( graph.keys.count() ) {
			double split_pos = posPts / length;
			QVariant split_value = graph.valueAt( split_pos );
			for ( int j = 0; j < graph.keys.count(); ++j ) {
				if ( graph.keys[j].x < split_pos )
					params[i]->graph.keys.append( AnimationKey( graph.keys[j].keyType, graph.keys[j].x / split_pos, graph.keys[j].y ) );
				else
					p_second[i]->graph.keys.append( AnimationKey( graph.keys[j].keyType, (graph.keys[j].x - split_pos) / (1.0 - split_pos), graph.keys[j].y ) );
			}
			p_second[i]->graph.keys.prepend( AnimationKey( p_second[i]->graph.keys.first().keyType, 0.0, split_value.toDouble() ) );
			params[i]->graph.keys.append( AnimationKey( params[i]->graph.keys.last().keyType, 1.0, split_value.toDouble() ) );
		}
	}
}
