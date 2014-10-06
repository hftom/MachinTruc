#include <movit/deconvolution_sharpen_effect.h>
#include "vfx/gldeconvolutionsharpen.h"



GLDeconvolutionSharpen::GLDeconvolutionSharpen( QString id, QString name ) : GLFilter( id, name )
{
	R = addParameter( "R", tr("Deconvolution radius:"), Parameter::PINT, 3, 1, 5, false );
	circleRadius = addParameter( "circleRadius", tr("Circle radius:"), Parameter::PDOUBLE, 2.0, 0.0, 5.0, true );
	gaussianRadius = addParameter( "gaussianRadius", tr("Gaussian radius:"), Parameter::PDOUBLE, 0.0, 0.0, 5.0, true );
	correlation = addParameter( "correlation", tr("Correlation:"), Parameter::PDOUBLE, 0.95, 0.0, 0.99, true );
	noise = addParameter( "noise", tr("Noise:"), Parameter::PDOUBLE, 0.01, 0.0, 0.1, true );
}



GLDeconvolutionSharpen::~GLDeconvolutionSharpen()
{
}



bool GLDeconvolutionSharpen::process( const QList<Effect*> &el, Frame *src, Profile *p )
{
	Q_UNUSED( p );
	double pts = src->pts();
	Effect *e = el[0];
	return e->set_float( "circle_radius", getParamValue( circleRadius, pts ).toFloat() )
		&& e->set_float( "gaussian_radius", getParamValue( gaussianRadius, pts ).toFloat() )
		&& e->set_float( "correlation", getParamValue( correlation, pts ).toFloat() )
		&& e->set_float( "noise", getParamValue( noise, pts ).toFloat() );
}



QList<Effect*> GLDeconvolutionSharpen::getMovitEffects()
{
	Effect *e = new DeconvolutionSharpenEffect();
	bool ok = e->set_int( "matrix_size", getParamValue( R ).toInt() );
	Q_UNUSED( ok );
	QList<Effect*> list;
	list.append( e );
	return list;
}



QString GLDeconvolutionSharpen::getDescriptor(  Frame *src, Profile *p  )
{
	Q_UNUSED( src );
	Q_UNUSED( p );
	return QString("%1 %2").arg( getIdentifier() ).arg( getParamValue( R ).toInt() );
}
