#include "glcover.h"



GLCover::GLCover( QString id, QString name ) : GLFilter( id, name )
{
	position = addParameter( "position", tr("Position:"), Parameter::PDOUBLE, 0.0, 0.0, 1.0, true );
	position->hidden = true;
	position->graph.keys.append( AnimationKey( AnimationKey::CURVE, 0, 0 ) );
	position->graph.keys.append( AnimationKey( AnimationKey::CURVE, 1, 1 ) );
	vertical = addBooleanParameter( "vertical", tr("Vertical"), 0 );
	direction = addBooleanParameter( "direction", tr("Opposite direction"), 0 );
	uncover = addBooleanParameter( "uncover", tr("Uncover"), 0 );
}



QString GLCover::getDescriptor( Frame *src, Profile *p )
{
	Q_UNUSED( src );
	Q_UNUSED( p );
	return QString("%1 %2 %3 %4").arg( getIdentifier() )
					.arg( getParamValue( vertical ).toInt() )
					.arg( getParamValue( direction ).toInt() )
					.arg( getParamValue( uncover ).toInt() );
}



bool GLCover::process( const QList<Effect*> &el, Frame *src, Frame *dst, Profile *p )
{
	Q_UNUSED( dst );
	Q_UNUSED( p );
	float pos = getParamValue( position, src->pts() ).toFloat();
	return el[0]->set_float( "position", getParamValue( uncover ).toInt() ? 1.0f - pos : pos );
}



QList<Effect*> GLCover::getMovitEffects()
{
	Effect *e = new MyCoverEffect();
	bool ok = e->set_int( "vertical", getParamValue( vertical ).toInt() )
				&& e->set_int( "direction", getParamValue( direction ).toInt() )
				&& e->set_int( "uncover", getParamValue( uncover ).toInt() );
	Q_UNUSED( ok );
	QList<Effect*> list;
	list.append( e );
	return list;
}
