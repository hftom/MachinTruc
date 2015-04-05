#include <QPainter>

#include "engine/util.h"

#include "gltext.h"



GLText::GLText( QString id, QString name ) : GLFilter( id, name )
{
	editor = addParameter( "editor", tr("Editor:"), Parameter::PSTRING, "", "", "", false );
	xOffsetPercent = addParameter( "xOffsetPercent", tr("X:"), Parameter::PDOUBLE, 0.0, -100.0, 100.0, true, "%" );
	yOffsetPercent = addParameter( "yOffsetPercent", tr("Y:"), Parameter::PDOUBLE, 0.0, -100.0, 100.0, true, "%" );
}



bool GLText::process( const QList<Effect*> &el, Frame *src, Profile *p )
{
	Q_UNUSED( p );
	double pts = src->pts();
	
	MyTextEffect *e = (MyTextEffect*)el[0];
	e->setText( getParamValue( editor ).toString() );	
	return e->set_float( "left", getParamValue( xOffsetPercent, pts ).toDouble() / 100.0 )
		&& e->set_float( "top", getParamValue( yOffsetPercent, pts ).toDouble() / 100.0 );
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
	QBrush myBrush( QColor(0,0,0,0) );
	int outline = 0;
	int align = 1;
		
	QStringList sl = currentText.split("\n");
	if ( sl.count() ) {
		QStringList desc = sl[0].split("|");
		if ( desc.count() == 8 ) {
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
				
			align = desc[5].toInt();
			
			int osize = desc[6].toInt();
			if ( osize > 0 ) {					
				QStringList oc = desc[7].split( "." );
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

			sl.takeFirst();
		}
	}	
	
	QImage *image = new QImage( 10, 10, QImage::Format_ARGB32_Premultiplied );
	QPainter painter;
	painter.begin( image );
	painter.setPen( myPen );
	painter.setBrush( myBrush );
	painter.setFont( myFont );
	QList<QRectF> br;
	QFontMetrics metrics = QFontMetrics( myFont );
	int h = sl.count() * metrics.lineSpacing();
	int w = 0;
	for ( int i = 0; i < sl.count(); ++i ) {
		QRectF minrect( 0, 0, 1, 1 );
		QRectF r = painter.boundingRect( minrect, Qt::AlignHCenter | Qt::AlignVCenter, sl[i] );
		if ( r.width() > w )
			w = r.width();
		br.append( r );
	}
	painter.end();
	
	w += outline + 10;
	h += outline + 10;
	double x, y;
	if ( w > iwidth ) {
		x = (iwidth - w) / 2.0;
		w = iwidth;
	}
	else
		x = ((double)outline + 10) / 2.0;
		
	if ( h > iheight ) {
		y = (iheight - h) / 2.0;
		h = iheight;
	}
	else
		y = ((double)outline + 10) / 2.0;
		
	delete image;
	image = new QImage( w, h, QImage::Format_ARGB32_Premultiplied );
	image->fill( QColor(0,0,0,0) );
	painter.begin( image );
	painter.setRenderHints( QPainter::Antialiasing | QPainter::TextAntialiasing );
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
