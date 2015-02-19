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
	rotateActive( false ),
	rotateLeft( 0 ),
	rotateTop( 0 ),
	left( 0 ),
	top( 0 )
{
	sizePercent = addParameter( "sizePercent", tr("Size:"), Parameter::PDOUBLE, 100.0, 0.0, 500.0, true, "%" );
	xOffsetPercent = addParameter( "xOffsetPercent", tr("X:"), Parameter::PDOUBLE, 0.0, -100.0, 100.0, true, "%" );
	yOffsetPercent = addParameter( "yOffsetPercent", tr("Y:"), Parameter::PDOUBLE, 0.0, -100.0, 100.0, true, "%" );
	rotateAngle = addParameter( "rotateAngle", tr("Rotation angle:"), Parameter::PDOUBLE, 0.0, -360.0, 360.0, true );
	softBorder = addParameter( "softBorder", tr("Soft border:"), Parameter::PINT, 2.0, 0, 10, false );
}



GLSize::~GLSize()
{
}



QString GLSize::getDescriptor( Frame *src, Profile *p )
{
	QString s;
	bool samesar = qAbs( p->getVideoSAR() - src->glSAR ) < 1e-3;

	if ( samesar && !sizePercent->graph.keys.count() && 100.0 == getParamValue( sizePercent ).toDouble() ) {
		resizeActive = false;
	}
	else {
		s += "Resize";
		resizeActive = true;
	}
	
	if ( !rotateAngle->graph.keys.count() && getParamValue( rotateAngle ).toDouble() == 0.0 )
		rotateActive = false;
	else {
		s += "Rotate";
		rotateActive = true;
	}

	src->glWidth = p->getVideoWidth();
	src->glHeight = p->getVideoHeight();
	src->glSAR = p->getVideoSAR();
	return s + "Padding";
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
	// scaled image size
	double sw = qMax( (double)src->glWidth * src->glSAR  / p->getVideoSAR() * zoom, 1.0 );
	double sh = qMax( (double)src->glHeight * zoom, 1.0 );
	// project size (screen)
	double pw = p->getVideoWidth();
	double ph = p->getVideoHeight();
	left = xoff * pw;
	top = yoff * ph;
	
	// We want to find the image subrect that will be visible.
	// We rotate and translate the screen,
	// using Eigen3 since it's already a Movit requirement.
	Vector2d vec1( -pw / 2.0, - ph / 2.0 );     // bottom left
	Vector2d vec2( vec1.x() + pw, vec1.y() );   // bottom right
	Vector2d vec3( vec1.x(), vec1.y() + ph );   // top left
	Vector2d vec4( vec2.x(), vec3.y() );        // top right
	Rotation2D<double> rot( -rad );
	Translation<double,2> trans( Vector2d(-left, -top) );
	vec1 = trans * rot * vec1;
	vec2 = trans * rot * vec2;
	vec3 = trans * rot * vec3;
	vec4 = trans * rot * vec4;
	
	// init the subrect to image size
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
	// adjust horizontal padding
	//left += x1 - keys.first();
	
	// do the same for y1 and y2
	map.clear();
	keys.clear();
	map.insert( vec1.y(), 0 );
	map.insert( vec2.y(), 0 );
	map.insert( vec3.y(), 0 );
	map.insert( vec4.y(), 0 );
	keys = map.keys();
	findPoints( y1, y2, keys.first(), keys.last() );
	//top += y1 - keys.first();
	
	double deltaCenterX = (x2 + x1) / 2.0;
	double deltaCenterY = (y2 + y1) / 2.0;

	// the origin was at the center
	x1 += sw / 2.0;
	x2 += sw / 2.0;
	// round up
	resizeOutputWidth = x2 - x1;
	if ( resizeOutputWidth < x2 - x1 || resizeOutputWidth == 0 )
		++resizeOutputWidth;
	resizeZoomX = sw / (double)resizeOutputWidth;
	resizeLeft = x1 / sw * src->glWidth;
	
	y1 += sh / 2.0;
	y2 += sh / 2.0;	
	resizeOutputHeight = y2 - y1;
	if ( resizeOutputHeight < x2 - x1 || resizeOutputHeight == 0 )
		++resizeOutputHeight;
	resizeZoomY = sh / (double)resizeOutputHeight;
	resizeTop = y1 / sh * src->glHeight;

	if ( rotateActive ) {
		rotateSize = sqrt( (double)resizeOutputWidth * (double)resizeOutputWidth + (double)resizeOutputHeight * (double)resizeOutputHeight ) + 1;
		rotateLeft = (rotateSize - (double)resizeOutputWidth) / 2.0;
		rotateTop = (rotateSize - (double)resizeOutputHeight) / 2.0;
		left -= (rotateSize - pw) / 2.0;
		top -= (rotateSize - ph) / 2.0;
	}
	else {
		left -= ((double)resizeOutputWidth - pw) / 2.0;
		top -= ((double)resizeOutputHeight - ph) / 2.0;
	}
	left += deltaCenterX;
	top += deltaCenterY;
	
	src->glWidth = pw;
	src->glHeight = ph;
	src->glSAR = p->getVideoSAR();
	
	if ( resizeActive ) {
		Effect *e = el[0];
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
		ok |= e->set_int( "width", rotateSize )
			&& e->set_int( "height", rotateSize )
			&& e->set_float( "top", rotateTop )
			&& e->set_float( "left", rotateLeft );
		++index;
		
		ok |= el[index]->set_float( "angle", rad )
			&& el[index]->set_float( "SAR", src->glSAR );
		++index;
	}
	
	Effect *e = el[index];
	return ok && e->set_int( "width", src->glWidth )
		&& e->set_int( "height", src->glHeight )
		&& e->set_float( "top", top )
		&& e->set_float( "left", left );
}



QList<Effect*> GLSize::getMovitEffects()
{
	QList<Effect*> list;
	if ( resizeActive )
		list.append( new ResampleEffect() );
	if ( rotateActive ) {
		list.append( new MySoftBorderEffect() );
		list.append( new PaddingEffect() );
		list.append( new MyRotateEffect() );
	}
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
	}
	else
		x1 = first;
	
	if ( x2 <= last ) {
		if ( x2 < first ) {
			x1 = x2 = 0;
			return;
		}
	}
	else
		x2 = last;
}

