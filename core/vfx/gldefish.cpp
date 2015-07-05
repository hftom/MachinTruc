#include "glborder.h"
#include "gldefish.h"



GLDefish::GLDefish( QString id, QString name ) : GLFilter( id, name )
{
	amount = addParameter( "amount", tr("Amount:"), Parameter::PDOUBLE, 2.0, 0.01, 10.0, true );
	scale = addParameter( "scale", tr("Scale:"), Parameter::PDOUBLE, 1.0, 1.0, 10.0, true );
}



bool GLDefish::process( const QList<Effect*> &el, double pts, Frame *src, Profile *p )
{
	Q_UNUSED( src );
	Q_UNUSED( p );
	return el[1]->set_float( "amount", getParamValue( amount, pts ).toDouble() )
		&& el[1]->set_float( "scale", getParamValue( scale, pts ).toDouble() );
}



QList<Effect*> GLDefish::getMovitEffects()
{
	QList<Effect*> list;
	Effect *border = new MyBorderEffect();
	RGBATuple col = RGBATuple( 0, 0, 0, 0 );
	bool ok = border->set_vec4( "color", (float*)&col );
	Q_UNUSED( ok );
	list.append( border );
	list.append( new MyDefishEffect() );
	return list;
}
