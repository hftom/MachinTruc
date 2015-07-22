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
	motionBlur = addParameter( "motionBlur", tr("Motion blur:"), Parameter::PDOUBLE, 0.0, 0.0, 1.0, false );
}



QString GLCover::getDescriptor( double pts, Frame *src, Profile *p )
{
	Q_UNUSED( pts );
	Q_UNUSED( src );
	Q_UNUSED( p );
	return QString("%1 %2 %3 %4 %5").arg( getIdentifier() )
					.arg( getParamValue( vertical ).toInt() )
					.arg( getParamValue( direction ).toInt() )
					.arg( getParamValue( uncover ).toInt() )
					.arg( getParamValue( motionBlur ).toFloat() > 0.0f );
}



bool GLCover::process( const QList<Effect*> &el, double pts, Frame *first, Frame *second, Profile *p )
{
	Q_UNUSED( first );
	Q_UNUSED( second );
	Q_UNUSED( p );
	float pos = getParamValue( position, pts ).toFloat();
	float blur = getParamValue( motionBlur ).toFloat();
	Effect *e = el[0];
	
	if ( first->glOVD && getParamValue( uncover ).toInt() ) {
		if ( getParamValue( vertical ).toInt() ) {
			if ( getParamValue( direction ).toInt() )
				first->glOVDTransformList.append( FilterTransform( FilterTransform::TRANSLATE, 0, pos * first->glHeight ) );
			else
				first->glOVDTransformList.append( FilterTransform( FilterTransform::TRANSLATE, 0, -pos * first->glHeight ) );
		}
		else {
			if ( getParamValue( direction ).toInt() )
				first->glOVDTransformList.append( FilterTransform( FilterTransform::TRANSLATE, pos * first->glWidth, 0 ) );
			else
				first->glOVDTransformList.append( FilterTransform( FilterTransform::TRANSLATE, -pos * first->glWidth, 0 ) );
		}
	}
	
	if ( blur > 0.0f ) {
		float ppos = getParamValue( position, qMax(0.0, pts - p->getVideoFrameDuration()) ).toFloat();
		float texSize[2] = { 1.0f / first->glWidth, 1.0f / first->glHeight };
		int loop;
		if ( getParamValue( vertical ).toInt() ) {
			loop = (pos - ppos) / texSize[1] * blur;
			texSize[1] *= -1.0f;
			texSize[0] = 0;
		}
		else {
			loop = (pos - ppos) / texSize[0] * blur;
			texSize[1] = 0;
		}
		if ( getParamValue( direction ).toInt() ) {
			texSize[0] *= -1.0f;
			texSize[1] *= -1.0f;
		}
		
		//qDebug() << "loop" << loop;
		
		return e->set_float( "position", getParamValue( uncover ).toInt() ? 1.0f - pos : pos )
			&& e->set_float( "loop", qMax(loop, 1) )
			&& e->set_vec2( "texSize", texSize );
	}

	return e->set_float( "position", getParamValue( uncover ).toInt() ? 1.0f - pos : pos )
		&& e->set_float( "loop", 1 );
}



QList<Effect*> GLCover::getMovitEffects()
{
	Effect *e = new MyCoverEffect();
	bool ok = e->set_int( "vertical", getParamValue( vertical ).toInt() )
				&& e->set_int( "direction", getParamValue( direction ).toInt() )
				&& e->set_int( "uncover", getParamValue( uncover ).toInt() )
				&& e->set_float( "loop", getParamValue( motionBlur ).toFloat() > 0.0f ? 2.0 : 1.0 );
	Q_UNUSED( ok );
	QList<Effect*> list;
	list.append( e );
	return list;
}
