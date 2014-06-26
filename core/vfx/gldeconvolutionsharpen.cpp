#include <movit/deconvolution_sharpen_effect.h>
#include "vfx/gldeconvolutionsharpen.h"



GLDeconvolutionSharpen::GLDeconvolutionSharpen( QString id, QString name ) : GLFilter( id, name )
{
	R = 3;
	circleRadius = 2.0;
	gaussianRadius = 0.0;
	correlation = 0.95;
	noise = 0.01;
	addParameter( tr("Deconvolution radius:"), PINT, 1, 5, true, &R );
	addParameter( tr("Circle radius:"), PFLOAT, 0.0, 5.0, true, &circleRadius );
	addParameter( tr("Gaussian radius:"), PFLOAT, 0.0, 5.0, true, &gaussianRadius );
	addParameter( tr("Correlation:"), PFLOAT, 0.0, 0.99, true, &correlation );
	addParameter( tr("Noise:"), PFLOAT, 0.0, 0.1, true, &noise );
}



GLDeconvolutionSharpen::~GLDeconvolutionSharpen()
{
}



bool GLDeconvolutionSharpen::process( const QList<Effect*> &el, Frame *src, Profile *p )
{
	Q_UNUSED( src );
	Q_UNUSED( p );
	return el.at(0)->set_float( "circle_radius", circleRadius )
		&& el.at(0)->set_float( "gaussian_radius", gaussianRadius )
		&& el.at(0)->set_float( "correlation", correlation )
		&& el.at(0)->set_float( "noise", noise );
}



QList<Effect*> GLDeconvolutionSharpen::getMovitEffects()
{
	Effect *e = new DeconvolutionSharpenEffect();
	e->set_int( "matrix_size", R );
	QList<Effect*> list;
	list.append( e );
	return list;
}



QString GLDeconvolutionSharpen::getDescriptor()
{
	return QString("%1 %2").arg( getFilterName() ).arg( R );
}
