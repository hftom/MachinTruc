#include <movit/resample_effect.h>

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
	centerOffsetY(0)
{
	sizePercent = addParameter( "sizePercent", tr("Size:"), Parameter::PINPUTDOUBLE, 100.0, 0.0, 10000.0, true, "%" );
	xOffset = addParameter( "xOffset", tr("X offset:"), Parameter::PINPUTDOUBLE, 0.0, -110.0, 110.0, true, "%" );
	yOffset = addParameter( "yOffset", tr("Y offset:"), Parameter::PINPUTDOUBLE, 0.0, -110.0, 110.0, true, "%" );
	rotateAngle = addParameter( "rotateAngle", tr("Rotation angle:"), Parameter::PDOUBLE, 0.0, -360.0, 360.0, true );
	blurFiller = addBooleanParameter("blurFiller", tr("Fill background with blur (no effect if rotation):"), 0);
	//softBorder = addParameter( "softBorder", tr("Soft border:"), Parameter::PINT, 2, 1, 10, false );
}



GLSize::~GLSize()
{
}



QString GLSize::getDescriptor( double pts, Frame *src, Profile *p )
{
	preProcess(pts, src, p);

	QString s;
	bool samesar = qAbs( p->getVideoSAR() - src->glSAR ) < 1e-3;

	double zoom = p->getVideoWidth() / (double)src->glWidth * getParamValue( sizePercent ).toDouble() / 100.0;
	if ( samesar && !sizePercent->graph.keys.count() && zoom == 1.0 ) {
		resizeActive = false;
	}
	else {
		s += "Resize";
		resizeActive = true;
	}
	
	blurFillerActive = false;
	softBorderActive = false;
	if ( !rotateAngle->graph.keys.count() && getParamValue( rotateAngle ).toDouble() == 0.0 ) {
		rotateActive = false;
		if (getParamValue(blurFiller).toInt()) {
			blurFillerActive = true;
			s += "FillBlur";
			if (sizePercent->graph.keys.count()) {
				softBorderActive = true;
				s += "SoftBorder";
			}
		}
		else {
			s += "Padding";
		}
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



void GLSize::ovdUpdate( QString type, QVariant val )
{
	if ( type == "translate" ) {
		ovdOffset = val;
	}
	else if ( type == "scale" ) {
		ovdScale = val;
	}
}



bool GLSize::process( const QList<Effect*> &el, double pts, Frame *src, Profile *p )
{
	preProcess(pts, src, p);

	bool ok = true;
	int index = 0;

	// screen size (project)
	double pw = p->getVideoWidth();
	double ph = p->getVideoHeight();
	double psar = p->getVideoSAR();
	bool vertical = (double)src->glWidth / (double)src->glHeight < pw / ph;
	
	if (!ovdScale.isNull()) {
		QPointF pos = ovdScale.toPointF();
		if ( !sizePercent->hidden && !sizePercent->graph.keys.count() ) {
			if (vertical) {
				sizePercent->value = pos.x() / (ph / (double)src->glHeight);
			}
			else {
				sizePercent->value = pos.x() / (pw / (double)src->glWidth);
			}
		}
		
		ovdScale = QVariant();
	}
	
	double rad = getParamValue( rotateAngle, pts ).toDouble() * M_PI / 180.0;
	double zoom = 1.0;
	if (vertical) {
		zoom = ph / (double)src->glHeight * getParamValue( sizePercent, pts ).toDouble() / 100.0;
	}
	else {
		zoom = pw / (double)src->glWidth * getParamValue( sizePercent, pts ).toDouble() / 100.0;
	}
	
	if (!ovdOffset.isNull()) {
		QPointF pos = ovdOffset.toPointF();
		if ( !xOffset->hidden && !xOffset->graph.keys.count() ) {
			xOffset->value = pos.x() * 100.0 / pw;
		}
		if ( !yOffset->hidden && !yOffset->graph.keys.count() ) {
			yOffset->value = pos.y() * 100.0 / ph;
		}
		
		ovdOffset = QVariant();
	}
	
	double left = pw * getParamValue( xOffset, pts ).toDouble() / 100.0;
	double top = ph * getParamValue( yOffset, pts ).toDouble() / 100.0;
	// scaled centered image
	double sw = qMax( (double)src->glWidth * src->glSAR  / psar * zoom, 1.0 );
	double sh = qMax( (double)src->glHeight * zoom, 1.0 );
	
	double imageWidth = resizeOutputWidth = src->glWidth;
	double imageHeight = resizeOutputHeight = src->glHeight;
	
	if ( ovdEnabled() ) {
		src->glOVD = FilterTransform::SCALE | FilterTransform::TRANSLATE;
		src->glOVDRect = QRectF( -imageWidth / 2.0, -imageHeight / 2.0, imageWidth, imageHeight );
	}
	if ( src->glOVD ) {
		if ( qAbs( src->glSAR - psar ) > 1e-3 ) {
			src->glOVDTransformList.append( FilterTransform( FilterTransform::NERATIO, src->glSAR / psar, 1.0 ) );
		}
		src->glOVDTransformList.append( FilterTransform( FilterTransform::SCALE, qMax(1e-6, zoom), qMax(1e-6, zoom) ) );
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
	
	if (softBorderActive) {
		++index;
	}
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
		//ok |= el[index]->set_float( "borderSize", getParamValue( softBorder ).toInt() );
		++index;
		
		Effect *e = el[index];
		ok |= e->set_float( "angle", rad )
			&& e->set_float( "SAR", psar )
			&& e->set_int( "width", pw )
			&& e->set_int( "height", ph )
			&& e->set_float( "top", top )
			&& e->set_float( "left", left )
			&& e->set_float( "centerOffsetX", centerOffsetX )
			&& e->set_float( "centerOffsetY", centerOffsetY );
			
	}
	else {
		if (blurFillerActive) {
			MyBlurFillerEffect *e = (MyBlurFillerEffect*)el[index];
			e->setPadding(pw, ph, top, left);
			ok |= true;
		}
		else {
			Effect *e = el[index];
			ok |= e->set_int( "width", pw )
				&& e->set_int( "height", ph )
				&& e->set_float( "top", top )
				&& e->set_float( "left", left );
		}
	}

	src->glWidth = pw;
	src->glHeight = ph;
	src->glSAR = psar;
	
	return ok;
}



QList<Effect*> GLSize::getMovitEffects()
{
	QList<Effect*> list;
	if (softBorderActive) {
		list.append( new MySoftBorderEffect() );
	}
	if ( resizeActive )
		list.append( new ResampleEffect() );
	if ( rotateActive ) {
		list.append( new MySoftBorderEffect() );
		//list.append( new PaddingEffect() );
		list.append( new MyRotateEffect() );
	}
	else {
		if (blurFillerActive) {
			list.append( new MyBlurFillerEffect() );
		}
		else {
			list.append( new PaddingEffect() );
		}
	}

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

