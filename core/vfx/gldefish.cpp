#include "gldefish.h"



GLDefish::GLDefish( QString id, QString name ) : GLFilter( id, name )
{
	factor = addParameter( "factor", tr("Factor:"), Parameter::PDOUBLE, 2.0, 0.0, 10.0, true );
	scale = addParameter( "scale", tr("Scale:"), Parameter::PDOUBLE, 1.0, 1.0, 10.0, true );
}



bool GLDefish::process( const QList<Effect*> &el, Frame *src, Profile *p )
{
	Q_UNUSED( p );
	return el[0]->set_float( "factor", getParamValue( factor, src->pts() ).toDouble() )
		&& el[0]->set_float( "scale", getParamValue( scale, src->pts() ).toDouble() );
}



QList<Effect*> GLDefish::getMovitEffects()
{
	QList<Effect*> list;
	list.append( new MyDefishEffect() );
	return list;
}
