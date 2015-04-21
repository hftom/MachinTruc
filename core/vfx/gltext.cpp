#include <QPainter>

#include "engine/util.h"

#include "gltext.h"



GLText::GLText( QString id, QString name ) : GLFilter( id, name )
{
	editor = addParameter( "editor", tr("Editor:"), Parameter::PSTRING, "", "", "", false );
	opacity = addParameter( "opacity", tr("Opacity:"), Parameter::PDOUBLE, 1.0, 0.0, 1.0, true );
	xOffset = addParameter( "xOffset", tr("X:"), Parameter::PINPUTDOUBLE, 0.0, -10000.0, 10000.0, true );
	yOffset = addParameter( "yOffset", tr("Y:"), Parameter::PINPUTDOUBLE, 0.0, -10000.0, 10000.0, true );
}



void GLText::ovdUpdate( QString type, QVariant val )
{
	if ( type == "translate" ) {
		QPointF pos = val.toPointF();
		xOffset->value = pos.x();
		yOffset->value = pos.y();
	}
}



bool GLText::process( const QList<Effect*> &el, Frame *src, Profile *p )
{
	Q_UNUSED( p );
	double pts = src->pts();
	
	MyTextEffect *e = (MyTextEffect*)el[0];
	bool ok = e->set_float( "left", getParamValue( xOffset, pts ).toDouble() )
		&& e->set_float( "top", getParamValue( yOffset, pts ).toDouble() )
		&& e->set_float( "opacity", getParamValue( opacity, pts ).toDouble() );
	e->setText( getParamValue( editor ).toString(), src->glWidth, src->glHeight );
		
	if ( ovdEnabled() ) {
		src->glOVD = FilterTransform::TRANSLATE;
		double w = e->getImageWidth(), h = e->getImageHeight();
		src->glOVDRect = QRectF( -w / 2.0, -h / 2.0, w, h );
		src->glOVDTransformList.append( FilterTransform( FilterTransform::TRANSLATE, getParamValue( xOffset, pts ).toDouble(), getParamValue( yOffset, pts ).toDouble() ) );
	}
	
	return ok;
}



QList<Effect*> GLText::getMovitEffects()
{
	QList<Effect*> list;
	list.append( new MyTextEffect() );
	return list;
}



QImage* MyTextEffect::drawImage()
{
	QFont myFont;
	QPen myPen;
	myPen.setJoinStyle( Qt::RoundJoin );
	QBrush myBrush( QColor(0,0,0,0) );
	QColor backgroundColor(0,0,0,0);
	int outline = 0;
	int align = 1;
		
	QStringList sl = currentText.split("\n");
	while ( !sl.isEmpty() ) {
		if ( sl.last().trimmed().isEmpty() )
			sl.takeLast();
		else
			break;
	}
	if ( sl.count() ) {
		QStringList desc = sl[0].split("|");
		if ( desc.count() == 9 ) {
			myFont.fromString( desc[0] );
			myFont.setPointSize( desc[1].toInt() );
			myFont.setBold( desc[2].toInt() );
			myFont.setItalic( desc[3].toInt() );

			QStringList fc = desc[4].split( "." );
			if ( fc.count() == 2 ) {
				QColor col;
				col.setNamedColor( fc[ 0 ] );
				col.setAlpha( fc[ 1 ].toInt() );
				myPen.setColor( col );
				myBrush.setColor( col );
			}
			
			QStringList bc = desc[5].split( "." );
			if ( bc.count() == 2 ) {
				backgroundColor.setNamedColor( bc[ 0 ] );
				backgroundColor.setAlpha( bc[ 1 ].toInt() );
			}
				
			align = desc[6].toInt();
			
			int osize = desc[7].toInt();
			if ( osize > 0 ) {					
				QStringList oc = desc[8].split( "." );
				if ( oc.count() == 2 ) {
					outline = osize;
					myPen.setWidth( osize );
					myFont.setStyleStrategy( QFont::ForceOutline );
					QColor col;
					col.setNamedColor( oc[ 0 ] );
					col.setAlpha( oc[ 1 ].toInt() );
					myPen.setColor( col );
				}
			}
		}
		sl.takeFirst();
	}	
	
	QImage *image = new QImage( 10, 10, QImage::Format_ARGB32_Premultiplied );
	QPainter painter;
	painter.begin( image );
	painter.setPen( myPen );
	painter.setBrush( myBrush );
	painter.setFont( myFont );
	QList<QRectF> br;
	QFontMetrics metrics( myFont );
	int h = sl.count() * metrics.lineSpacing();
	int w = 0;
	for ( int i = 0; i < sl.count(); ++i ) {
		QRectF minrect( 0, 0, 1, 1 );
		QRectF r = painter.boundingRect( minrect, Qt::AlignHCenter | Qt::AlignVCenter, sl[i] );
		if ( r.width() > w )
			w = r.width();
		br.append( r );
	}
	QRectF minrect( 0, 0, 1, 1 );
	int margin = qMax( painter.boundingRect( minrect, Qt::AlignHCenter | Qt::AlignVCenter, "M" ).width() / 3.0, 3.0 );
	
	painter.end();
	
	double x = ((double)outline + margin * 2) / 2.0;
	double y = x;
	w += 2 * x;
	h += 2 * y;
	if ( w > iwidth ) {
		x -= (w - iwidth) / 2.0;
		w = iwidth;
	}
	if ( h > iheight ) {
		y -= (h - iheight) / 2.0;
		h = iheight;
	}		
		
	delete image;
	image = new QImage( w, h, QImage::Format_ARGB32_Premultiplied );
	image->fill( QColor(0,0,0,0) );
	painter.begin( image );
	painter.setRenderHints( QPainter::Antialiasing | QPainter::TextAntialiasing );
	if ( backgroundColor.alpha() > 0 ) {
		painter.setPen( QColor(0,0,0,0) );
		painter.setBrush( backgroundColor );
		painter.drawRect( 1, 1, w - 2, h - 2 );
	}	
	painter.setPen( myPen );
	painter.setBrush( myBrush );
	painter.setFont( myFont );

	for ( int i = 0; i < sl.count(); ++i ) {
		QPointF point( 0, y + metrics.ascent() );
		switch ( align ) {
			case 2: {
				point.setX( (double)w / 2.0 - br[i].width() / 2.0 );
				break;
			}
			case 3: {
				point.setX( w - x - br[i].width() );
				break;
			}
			default: {
				point.setX( x );
				break;
			}
		}
		if ( outline ) {
			QPainterPath myPath;
			myPath.addText( point, myFont, sl[i] );
			painter.drawPath( myPath );
		}
		else
			painter.drawText( point, sl[i] );
		y += metrics.lineSpacing();
	}
	painter.end();
	
	return image;
}
