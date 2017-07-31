#include "vfx/glblur.h"



GLBlur::GLBlur( QString id, QString name ) : GLFilter( id, name )
{
	amount = addParameter( "amount", tr("Amount:"), Parameter::PDOUBLE, 1.0, 0.0, 10.0, true, "%" );
}



GLBlur::~GLBlur()
{
}



bool GLBlur::process( const QList<Effect*> &el, double pts, Frame *src, Profile *p )
{
	Q_UNUSED( p );
	bool ok = el.at(0)->set_float( "radius", src->glWidth * getParamValue( amount, pts ).toDouble() / 100.0 );
	return ok;
}



QList <Effect*> GLBlur::getMovitEffects()
{
	QList<Effect*> list;
	list.append( new BlurEffect() );
	return list;
}
