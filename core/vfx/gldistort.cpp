#include <movit/resample_effect.h>
#include <movit/padding_effect.h>
#include "vfx/gldistort.h"



GLDistort::GLDistort( QString id, QString name ) : GLFilter( id, name )
{
	ratio = addParameter( "ratio", tr("Ratio:"), Parameter::PDOUBLE, 1.0, 0.1, 10.0, true );
}



GLDistort::~GLDistort()
{
}



bool GLDistort::process( const QList<Effect*> &el, double pts, Frame *src, Profile *p )
{
	int w, h;
	float top = 0.0f;
	float left = 0.0f;
	
	double sar = getParamValue( ratio ).toFloat();
	
	if (sar < 1) {
		w = src->glWidth * sar;
		h = src->glHeight;
		left = (float)(src->glWidth - w) / 2.0f;
	}
	else {
		w = src->glWidth;
		h = src->glHeight / sar;
		top = (float)(src->glHeight - h) / 2.0f;
	}
	
	return el[0]->set_int( "width", w )
		&& el[0]->set_int( "height", h )
		&& el[1]->set_int("width", src->glWidth)
		&& el[1]->set_int("height", src->glHeight)
		&& el[1]->set_float("top", top)
		&& el[1]->set_float("left", left);
	
	return true;
}



QList<Effect*> GLDistort::getMovitEffects()
{
	QList<Effect*> list;
	list.append( new ResampleEffect() );
	list.append( new PaddingEffect() );
	return list;
}
