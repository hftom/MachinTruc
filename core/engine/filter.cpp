#include <QDebug>

#include "filter.h"



Filter::Filter( QString id, QString name )
{
	identifier = id;
	filterName = name;
	refCount = 0;
	use();
}



Filter::~Filter()
{
	while ( !parameters.isEmpty() )
		delete parameters.takeFirst();
}



void Filter::use()
{
	rcMutex.lock();
	++refCount;
	rcMutex.unlock();
}



void Filter::release()
{
	rcMutex.lock();
	--refCount;
	//qDebug() << "Filter::release" << refCount << filterName << this;
	rcMutex.unlock();
	
	if ( !refCount ) {
		//qDebug() << filterName << "delete";
		delete this;
	}
}



Parameter* Filter::addParameter( QString name, int type, QVariant def, QVariant min, QVariant max, bool keyframeable, const QString &suffix )
{
	Parameter *param = new Parameter();
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



QVariant Filter::getParamValue( Parameter *p, double pts )
{
	if ( !p->graph.keys.count() ) {
		return  p->value;
	}
	
	double range = qAbs( -p->min.toDouble() + p->max.toDouble() );
	double time = (pts - posInTrack) / length;
	return (range * p->graph.valueAt( time )) + p->min.toDouble();
}
