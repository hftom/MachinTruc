#include <movit/unsharp_mask_effect.h>
#include "vfx/glsharpen.h"



GLSharpen::GLSharpen( QString id, QString name ) : GLFilter( id, name )
{
	amount = addParameter( "amount", tr("Amount:"), Parameter::PDOUBLE, 0.5, 0.0, 2.0, true );
	blur = addParameter( "blur", tr("Blur:"), Parameter::PDOUBLE, 0.15, 0.0, 0.5, true, "%" );
}



GLSharpen::~GLSharpen()
{
}



bool GLSharpen::process( const QList<Effect*> &el, double pts, Frame *src, Profile *p )
{
	Q_UNLIKELY( src );
	Q_UNUSED( p );
	return el.at(0)->set_float( "amount", getParamValue( amount, pts ).toFloat() )
	&& el.at(0)->set_float( "radius", src->glWidth * getParamValue( blur, pts ).toDouble() / 100.0 );
}



QList<Effect*> GLSharpen::getMovitEffects()
{
	QList<Effect*> list;
	list.append( new UnsharpMaskEffect() );
	return list;
}
