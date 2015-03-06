#include "glhardcut.h"



GLHardCut::GLHardCut( QString id, QString name ) : GLFilter( id, name )
{
	position = addParameter( "position", tr("Cut position:"), Parameter::PDOUBLE, 0.5, 0.0, 1.0, false );
}



bool GLHardCut::process( const QList<Effect*> &el, Frame *src, Frame *dst, Profile *p )
{
	Q_UNUSED( dst );
	Q_UNUSED( p );
	return el[0]->set_float( "show_second", getNormalizedTime( src->pts() ) > getParamValue( position ).toDouble() ? 1 : 0 );
}



QList<Effect*> GLHardCut::getMovitEffects()
{
	QList<Effect*> list;
	list.append( new MyHardCutEffect() );
	return list;
}
