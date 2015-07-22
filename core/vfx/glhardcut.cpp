#include "glhardcut.h"



GLHardCut::GLHardCut( QString id, QString name ) : GLFilter( id, name )
{
	position = addParameter( "position", tr("Cut position:"), Parameter::PDOUBLE, 0.5, 0.0, 1.0, false );
}



bool GLHardCut::process( const QList<Effect*> &el, double pts, Frame *first, Frame *second, Profile *p )
{
	Q_UNUSED( first );
	Q_UNUSED( second );
	Q_UNUSED( p );
	return el[0]->set_float( "show_second", getNormalizedTime( pts ) > getParamValue( position ).toDouble() ? 1 : 0 );
}



QList<Effect*> GLHardCut::getMovitEffects()
{
	QList<Effect*> list;
	list.append( new MyHardCutEffect() );
	return list;
}
