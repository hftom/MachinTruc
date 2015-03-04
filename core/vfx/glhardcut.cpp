#include "glhardcut.h"



GLHardCut::GLHardCut( QString id, QString name ) : GLFilter( id, name )
{
	position = addParameter( "position", tr("Position:"), Parameter::PINT, 0, 0, 2, false );
}



bool GLHardCut::process( const QList<Effect*> &el, Frame *src, Frame *dst, Profile *p )
{
	Q_UNUSED( dst );
	Q_UNUSED( p );
	double pos = getParamValue( position ).toDouble() / 2.0;
	double time = getNormalizedTime( src->pts() );
	return el[0]->set_float( "show_second", time > pos ? 1 : 0 );
}



QList<Effect*> GLHardCut::getMovitEffects()
{
	QList<Effect*> list;
	list.append( new MyHardCutEffect() );
	return list;
}
