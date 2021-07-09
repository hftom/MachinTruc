#include <QDebug>

#include "filter.h"



Filter::Filter( QString id, QString name )
	: identifier( id ),
	filterName( name ),
	posInTrack( 0 ),
	length( 0 ),
	posOffset( 0 ),
	snap( SNAPALL ),
	showOVD( false )
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
	param->hidden = false;
	param->suffix = suffix;
	parameters.append( param );
	
	return param;
}



Parameter* Filter::addBooleanParameter( QString id, QString name, QVariant def )
{
	Parameter *param = new Parameter();
	param->id = id;
	param->name = name;
	param->type = Parameter::PBOOL;
	param->defValue = def;
	param->min = 0;
	param->max = 1;
	param->value = def;
	param->keyframeable = false;
	param->hidden = false;
	parameters.append( param );
	
	return param;
}



void Filter::prependLastParameter()
{
	Parameter *p = parameters.takeLast();
	parameters.prepend( p );
}



void Filter::removeParameters( QList<Parameter*> *params )
{
	for ( int i = 0; i < params->count(); ++i ) {
		Parameter *p = params->at( i );
		for ( int j = 0; j < parameters.count(); ++j ) {
			if ( parameters.at( j ) == p ) {
				delete parameters.takeAt( j );
				break;
			}
		}
	}
}



double Filter::getNormalizedTime( double pts )
{
	return (pts - (posInTrack + posOffset) ) / length;
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
			if (p_second[i]->graph.keys.count()) {
				p_second[i]->graph.keys.prepend( AnimationKey( p_second[i]->graph.keys.first().keyType, 0.0, split_value.toDouble() ) );
			}
			if (params[i]->graph.keys.count()) {
				params[i]->graph.keys.append( AnimationKey( params[i]->graph.keys.last().keyType, 1.0, split_value.toDouble() ) );
			}
		}
	}
}



void Filter::duplicateFilter( QSharedPointer<Filter> f )
{
	f->setPosition( getPosition() );
	f->setLength( getLength() );
	f->setPositionOffset( getPositionOffset() );
	QList<Parameter*> params = getParameters();
	QList<Parameter*> pf = f->getParameters();
	for ( int i = 0; i < params.count(); ++i ) {
		Parameter *p = params[i];
		Parameter *p1 = pf[i];
		p1->value = p->value;
		for ( int j = 0; j < p->graph.keys.count(); ++j ) {
			p1->graph.keys.append( AnimationKey( p->graph.keys[j].keyType, p->graph.keys[j].x, p->graph.keys[j].y ) );
		}
	}
}
