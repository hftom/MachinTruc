#include "glpush.h"



GLPush::GLPush( QString id, QString name ) : GLFilter( id, name )
{
	position = addParameter( "position", tr("Position:"), Parameter::PDOUBLE, 0.0, 0.0, 1.0, true );
	position->graph.keys.append( AnimationKey( AnimationKey::CURVE, 0, 0 ) );
	position->graph.keys.append( AnimationKey( AnimationKey::CURVE, 1, 1 ) );
	position->hidden = true;
	vertical = addBooleanParameter( "vertical", tr("Vertical"), 0 );
	direction = addBooleanParameter( "direction", tr("Opposite direction"), 0 );
}



QString GLPush::getDescriptor( Frame *src, Profile *p )
{
	Q_UNUSED( src );
	Q_UNUSED( p );
	return QString("%1 %2 %3").arg( getIdentifier() ).arg( getParamValue( vertical ).toInt() ).arg( getParamValue( direction ).toInt() );
}



bool GLPush::process( const QList<Effect*> &el, Frame *src, Frame *dst, Profile *p )
{
	Q_UNUSED( dst );
	Q_UNUSED( p );
	return el[0]->set_float( "position", getParamValue( position, src->pts() ).toFloat() );
}



QList<Effect*> GLPush::getMovitEffects()
{
	Effect *e = new MyPushEffect();
	bool ok = e->set_int( "vertical", getParamValue( vertical ).toInt() )
				&& e->set_int( "direction", getParamValue( direction ).toInt() );
	Q_UNUSED( ok );
	QList<Effect*> list;
	list.append( e );
	return list;
}
