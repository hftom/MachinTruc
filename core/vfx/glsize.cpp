#include <movit/resample_effect.h>
#include <movit/resize_effect.h>
#include <movit/padding_effect.h>
#include <Eigen/Core>
#include <Eigen/Geometry>

#include "glsize.h"

using namespace Eigen;



GLSize::GLSize( QString id, QString name ) : GLFilter( id, name ),
	resizeActive( true ),
	resizeOutputWidth( 1 ),
	resizeOutputHeight( 1 ),
	resizeZoomX( 1 ),
	resizeZoomY( 1 ),
	resizeLeft( 0 ),
	resizeTop( 0 ),
	centerOffsetX(0),
	centerOffsetY(0),
	left( 0 ),
	top( 0 )
{
	sizePercent = addParameter( "sizePercent", tr("Size:"), Parameter::PDOUBLE, 100.0, 0.0, 500.0, true, "%" );
	xOffsetPercent = addParameter( "xOffsetPercent", tr("X:"), Parameter::PDOUBLE, 0.0, -100.0, 100.0, true, "%" );
	yOffsetPercent = addParameter( "yOffsetPercent", tr("Y:"), Parameter::PDOUBLE, 0.0, -100.0, 100.0, true, "%" );
	rotateAngle = addParameter( "rotateAngle", tr("Rotation angle:"), Parameter::PDOUBLE, 0.0, -360.0, 360.0, true );
	softBorder = addParameter( "softBorder", tr("Soft border:"), Parameter::PINT, 2, 1, 10, false );
}



GLSize::~GLSize()
{
}



QString GLSize::getDescriptor( Frame *src, Profile *p )
{
	QString s;
	bool samesar = qAbs( p->getVideoSAR() - src->glSAR ) < 1e-3;

	if ( samesar && !sizePercent->graph.keys.count() && getParamValue( sizePercent ).toDouble() == 100.0 ) {
		resizeActive = false;
	}
	else {
		s += "Resize";
		resizeActive = true;
	}
	
	if ( !rotateAngle->graph.keys.count() && getParamValue( rotateAngle ).toDouble() == 0.0 ) {
		rotateActive = false;
		s += "Padding";
	}
	else {
		s += "Rotate";
		rotateActive = true;
	}

	src->glWidth = p->getVideoWidth();
	src->glHeight = p->getVideoHeight();
	src->glSAR = p->getVideoSAR();
	return s;
}



bool GLSize::process( const QList<Effect*> &el, Frame *src, Profile *p )
{
	bool ok = true;
	int index = 0;
	
	double pts = src->pts();
	double rad = getParamValue( rotateAngle, pts ).toDouble() * M_PI / 180.0;
	double zoom = getParamValue( sizePercent, pts ).toDouble() / 100.0;
	double xoff = getParamValue( xOffsetPercent, pts ).toDouble() / 100.0;
	double yoff = getParamValue( yOffsetPercent, pts ).toDouble() / 100.0;
	// screen size (project)
	double pw = p->getVideoWidth();
	double ph = p->getVideoHeight();
	double psar = p->getVideoSAR();
	// scaled centered image
	double sw = qMax( (double)src->glWidth * src->glSAR  / psar * zoom, 1.0 );
	double sh = qMax( (double)src->glHeight * zoom, 1.0 );
	left = xoff * pw;
	top = yoff * ph;
	
	double imageWidth = resizeOutputWidth = src->glWidth;
	double imageHeight = resizeOutputHeight = src->glHeight;
	
	if ( ovdEnabled() ) {
		src->glOVD = true;
		src->glOVDRect = QRectF( -imageWidth / 2.0, -imageHeight / 2.0, imageWidth, imageHeight );
	}
	if ( src->glOVD ) {
		src->glOVDTransformList.append( FilterTransform( FilterTransform::SCALE, sw / src->glWidth, sh / src->glHeight ) );
		src->glOVDTransformList.append( FilterTransform( FilterTransform::ROTATE, rad ) );
		src->glOVDTransformList.append( FilterTransform( FilterTransform::TRANSLATE, left, top ) );
	}
	
	if ( resizeActive ) {
		// We translate and rotate the screen using Eigen3
		// since it's already a Movit requirement.
		// When an image edge is outside the screen,
		// we cut at the nearest screen corner.
		// This ensures that width/height are never more
		// than the screen diagonal.
		Vector2d vec1( -pw * psar / 2.0, - ph / 2.0 );     // bottom left
		Vector2d vec2( vec1.x() + pw * psar, vec1.y() );   // bottom right
		Vector2d vec3( vec1.x(), vec1.y() + ph );   // top left
		Vector2d vec4( vec2.x(), vec3.y() );        // top right
		Translation<double,2> trans( Vector2d(-left, top) ); // PaddingEffect origin is top_left so -(-top)
		Rotation2D<double> rot( -rad );
		vec1 = rot * trans * vec1;
		vec2 = rot * trans * vec2;
		vec3 = rot * trans * vec3;
		vec4 = rot * trans * vec4;
	
		// init the subrect to the scaled image size
		double x1 = -sw / 2.0, x2 = sw / 2.0, y1 = -sh / 2.0, y2 = sh / 2.0;
		// QMap sorts in ascending key order
		QMap<double,int> map;
		map.insert( vec1.x(), 0 );
		map.insert( vec2.x(), 0 );
		map.insert( vec3.x(), 0 );
		map.insert( vec4.x(), 0 );
		// get the sorted values
		QList<double> keys = map.keys();
		// find x1 and x2
		findPoints( x1, x2, keys.first(), keys.last() );
	
		// do the same for y1 and y2
		map.clear();
		keys.clear();
		map.insert( vec1.y(), 0 );
		map.insert( vec2.y(), 0 );
		map.insert( vec3.y(), 0 );
		map.insert( vec4.y(), 0 );
		keys = map.keys();
		findPoints( y1, y2, keys.first(), keys.last() );

		// compensate the center offset
		Vector2d newCenter( (x2 + x1) / 2.0, (y2 + y1) / 2.0 );
		newCenter = Rotation2D<double>( rad ) * newCenter;
		left += newCenter.x();
		top -= newCenter.y();

		// the origin was at the center
		x1 += sw / 2.0;
		x2 += sw / 2.0;
		y1 += sh / 2.0;
		y2 += sh / 2.0;	
		imageWidth = x2 - x1;
		imageHeight = y2 - y1;
		// round to nearest integer
		resizeOutputWidth = qMax( imageWidth + 0.5, 1.0 );
		resizeZoomX = sw / resizeOutputWidth;
		resizeLeft = x1 / sw * src->glWidth;
		resizeOutputHeight = qMax( imageHeight + 0.5, 1.0 );
		resizeZoomY = sh / resizeOutputHeight;
		resizeTop = (sh - y2) / sh * src->glHeight;
	}

	// compensate rounded resample size
	if ( rotateActive ) {
		centerOffsetX = (imageWidth - resizeOutputWidth) / 2.0;
		centerOffsetY = (resizeOutputHeight - imageHeight) / 2.0;
	}

	left -= (imageWidth - pw) / 2.0;
	top -= (imageHeight - ph) / 2.0;
	
	src->glWidth = pw;
	src->glHeight = ph;
	src->glSAR = psar;
	
	if ( resizeActive ) {
		Effect *e = el[index];
		ok = e->set_int( "width", resizeOutputWidth )
			&& e->set_int( "height", resizeOutputHeight )
			&& e->set_float( "zoom_x", resizeZoomX )
			&& e->set_float( "zoom_y", resizeZoomY )
			&& e->set_float( "left", resizeLeft )
			&& e->set_float( "top", resizeTop )
			&& e->set_float( "zoom_center_x", 0 )
			&& e->set_float( "zoom_center_y", 0 );
		++index;
	}
	
	if ( rotateActive ) {
		ok |= el[index]->set_float( "borderSize", getParamValue( softBorder ).toInt() );
		++index;
		
		Effect *e = el[index];
		ok |= e->set_float( "angle", rad )
			&& e->set_float( "SAR", src->glSAR )
			&& e->set_int( "width", src->glWidth )
			&& e->set_int( "height", src->glHeight )
			&& e->set_float( "top", top )
			&& e->set_float( "left", left )
			&& e->set_float( "centerOffsetX", centerOffsetX )
			&& e->set_float( "centerOffsetY", centerOffsetY );
			
	}
	else {
		Effect *e = el[index];
		ok |= e->set_int( "width", src->glWidth )
			&& e->set_int( "height", src->glHeight )
			&& e->set_float( "top", top )
			&& e->set_float( "left", left );
	}
	
	return ok;
}



QList<Effect*> GLSize::getMovitEffects()
{
	QList<Effect*> list;
	if ( resizeActive )
		list.append( new ResampleEffect() );
	if ( rotateActive ) {
		list.append( new MySoftBorderEffect() );
		//list.append( new PaddingEffect() );
		list.append( new MyRotateEffect() );
	}
	else
		list.append( new PaddingEffect() );

	return list;
}



void GLSize::findPoints( double &x1, double &x2, double first, double last )
{
	if ( x1 >= first ) {
		if ( x1 > last ) {
			x1 = x2 = 0;
			return;
		}
		//else x1 = x1
	}
	else
		x1 = qMax(first - 1.0, x1);
	
	if ( x2 <= last ) {
		if ( x2 < first ) {
			x1 = x2 = 0;
			return;
		}
		// else x2 = x2
	}
	else
		x2 = qMin(last + 1.0, x2);
}

