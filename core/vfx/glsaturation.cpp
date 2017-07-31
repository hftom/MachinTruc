#include <assert.h>
#include <vector>

#include <movit/effect_chain.h>

#include "vfx/glsaturation.h"



GLSaturation::GLSaturation( QString id, QString name ) : GLMask( id, name )
{
	saturation = addParameter( "saturation", tr("Saturation:"), Parameter::PDOUBLE, 1.0, 0.0, 5.0, true );

	GLMask::setParameters();
}



GLSaturation::~GLSaturation()
{
}



QString GLSaturation::getDescriptor( double pts, Frame *src, Profile *p  )
{
	Q_UNUSED( pts );
	Q_UNUSED( src );
	Q_UNUSED( p );
	return QString("%1 %2").arg( getIdentifier() ).arg( GLMask::getMaskDescriptor(pts, src, p) );
}



bool GLSaturation::process( const QList<Effect*> &el, double pts, Frame *src, Profile *p )
{
	Q_UNUSED( src );
	Q_UNUSED( p );
	bool ok = el.at(0)->set_float( "saturation", getParamValue( saturation, pts ).toFloat() );
	ok |= GLMask::processMask(pts, src, p);

	return ok;
}



QList<Effect*> GLSaturation::getMovitEffects()
{
	QList<Effect*> list;
	list.append( new PseudoEffect(this, new SaturationEffect) );
	return list;
}
