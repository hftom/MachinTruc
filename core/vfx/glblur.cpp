#include <movit/blur_effect.h>
#include "vfx/glblur.h"



GLBlur::GLBlur( QString id, QString name ) : GLMask( id, name )
{
	amount = addParameter( "amount", tr("Amount:"), Parameter::PDOUBLE, 1.0, 0.0, 10.0, true, "%" );
	
	GLMask::setParameters();
}



GLBlur::~GLBlur()
{
}



QString GLBlur::getDescriptor( double pts, Frame *src, Profile *p  )
{
	return QString("%1 %2").arg( getIdentifier() ).arg( GLMask::getMaskDescriptor(pts, src, p) );
}



bool GLBlur::process( const QList<Effect*> &el, double pts, Frame *src, Profile *p )
{
	bool ok = el.at(0)->set_float( "radius", src->glWidth * getParamValue( amount, pts ).toDouble() / 100.0 );
	ok |= GLMask::processMask(pts, src, p);

	return ok;
}



QList <Effect*> GLBlur::getMovitEffects()
{
	QList<Effect*> list;
	list.append( new PseudoEffect(this, new BlurEffect) );
	return list;
}
