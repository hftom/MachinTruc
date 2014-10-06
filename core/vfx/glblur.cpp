#include <movit/blur_effect.h>
#include "vfx/glblur.h"



GLBlur::GLBlur( QString id, QString name ) : GLFilter( id, name )
{
	amount = addParameter( "amount", tr("Amount:"), Parameter::PDOUBLE, 1.0, 0.0, 10.0, true, "%" );
}



GLBlur::~GLBlur()
{
}



bool GLBlur::process( const QList<Effect*> &el, Frame *src, Profile *p )
{
	Q_UNUSED( p );
	return el.at(0)->set_float( "radius", src->glWidth * getParamValue( amount, src->pts() ).toDouble() / 100.0 );
}



QList <Effect*> GLBlur::getMovitEffects()
{
	QList<Effect*> list;
	list.append( new BlurEffect() );
	return list;
}
