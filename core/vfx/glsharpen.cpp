#include <movit/unsharp_mask_effect.h>
#include "vfx/glsharpen.h"



GLSharpen::GLSharpen( QString id, QString name ) : GLFilter( id, name )
{
	amount = addParameter( "amount", tr("Amount:"), Parameter::PDOUBLE, 0.5, 0.0, 2.0, true );
	radius = addParameter( "radius", tr("Blur radius:"), Parameter::PDOUBLE, 3.0, 0.0, 10.0, true );
}



GLSharpen::~GLSharpen()
{
}



bool GLSharpen::process( const QList<Effect*> &el, double pts, Frame *src, Profile *p )
{
	Q_UNLIKELY( src );
	Q_UNUSED( p );
	return el.at(0)->set_float( "amount", getParamValue( amount, pts ).toFloat() )
		&& el.at(0)->set_float( "radius", getParamValue( radius, pts ).toFloat() );
}



QList<Effect*> GLSharpen::getMovitEffects()
{
	QList<Effect*> list;
	list.append( new UnsharpMaskEffect() );
	return list;
}
