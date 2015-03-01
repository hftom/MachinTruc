#include "glcover.h"



GLCover::GLCover( QString id, QString name ) : GLFilter( id, name )
{
	position = addParameter( "position", tr("Position:"), Parameter::PDOUBLE, 0.0, 0.0, 1.0, true );
	position->graph.keys.append( AnimationKey( AnimationKey::CURVE, 0, 0 ) );
	position->graph.keys.append( AnimationKey( AnimationKey::CURVE, 1, 1 ) );
	vertical = addParameter( "vertical", tr("Vertical:"), Parameter::PINT, 0, 0, 1, false );
	reversed = addParameter( "reversed", tr("Reversed:"), Parameter::PINT, 0, 0, 1, false );
}



QString GLCover::getDescriptor( Frame *src, Profile *p )
{
	Q_UNUSED( src );
	Q_UNUSED( p );
	return QString("%1 %2 %3").arg( getIdentifier() ).arg( getParamValue( vertical ).toInt() ).arg( getParamValue( reversed ).toInt() );
}



bool GLCover::process( const QList<Effect*> &el, Frame *src, Frame *dst, Profile *p )
{
	Q_UNUSED( dst );
	Q_UNUSED( p );
	return el[0]->set_float( "position", getParamValue( position, src->pts() ).toFloat() );
}



QList<Effect*> GLCover::getMovitEffects()
{
	Effect *e = new MyCoverEffect();
	bool ok = e->set_int( "vertical", getParamValue( vertical ).toInt() )
				&& e->set_int( "reversed", getParamValue( reversed ).toInt() );
	Q_UNUSED( ok );
	QList<Effect*> list;
	list.append( e );
	return list;
}
