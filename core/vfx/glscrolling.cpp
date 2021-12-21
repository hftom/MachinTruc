#include <QPainter>
#include <QPainterPath>

#include "engine/util.h"
#include "glscrolling.h"



GLScrolling::GLScrolling( QString id, QString name ) : GLFilter( id, name )
{
	editor = addParameter( "editor", tr("Editor:"), Parameter::PSTRING, "Arial,12,-1,5,50,0,0,0,0,0|100|0|0|#ffffff.255|#000000.0|1|0|#000000.255|0|50|50\nText", "", "", false );
	opacity = addParameter( "opacity", tr("Opacity:"), Parameter::PDOUBLE, 1.0, 0.0, 1.0, true );
	xOffset = addParameter( "xOffset", tr("X offset:"), Parameter::PINPUTDOUBLE, 0.0, -110.0, 110.0, true, "%" );
	yOffset = addParameter( "yOffset", tr("Y offset:"), Parameter::PINPUTDOUBLE, 0.0, -110.0, 110.0, true, "%" );

	yOffset->graph.keys.append( AnimationKey( AnimationKey::LINEAR, 0, 1 ) );
	yOffset->graph.keys.append( AnimationKey( AnimationKey::LINEAR, 1, 1 ) );
}



bool GLScrolling::process( const QList<Effect*> &el, double pts, Frame *src, Profile *p )
{
	double xof = src->glWidth * getParamValue( xOffset, pts ).toDouble() / 100.0;

	MyScrollingEffect *e = (MyScrollingEffect*)el[0];
	QString text = getParamValue( editor ).toString();
	e->setText(text, src->glWidth, src->glHeight);

	yOffset->graph.keys.first().y = 1;
	yOffset->graph.keys.last().y = 1;
	double yof = src->glHeight * getParamValue( yOffset, pts ).toDouble() / 100.0;

	bool ok = e->set_float( "left", xof )
		&& e->set_float( "top", yof )
		&& e->set_float( "opacity", getParamValue( opacity, pts ).toDouble() );
	
	return ok;
}



QList<Effect*> GLScrolling::getMovitEffects()
{
	QList<Effect*> list;
	list.append( new MyScrollingEffect() );
	return list;
}


#define MAXW 1920.0
#define MAXH 1080.0

QImage* MyScrollingEffect::drawImage()
{
	QFont myFont;
	myFont.setStyleHint(QFont::Helvetica, QFont::PreferAntialias);
	QPen myPen;
	myPen.setJoinStyle( Qt::RoundJoin );
	QPen myOutlinePen;
	myOutlinePen.setJoinStyle( Qt::RoundJoin );
	QBrush myBrush( QColor(0,0,0,0) );
	QColor backgroundColor(0,0,0,0);
	int outline = 0;
	int align = 1;
	
	int arrowType = 0, arrowSize = 0, arrowPos = 0;

	QStringList sl = currentText.split("\n");
	while ( !sl.isEmpty() ) {
		if ( sl.last().trimmed().isEmpty() )
			sl.takeLast();
		else
			break;
	}
	if ( sl.count() ) {
		QStringList desc = sl[0].split("|");
		if ( desc.count() >= 9 ) {
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
					myOutlinePen.setWidth( osize );
					//myFont.setStyleStrategy( QFont::ForceOutline );
					QColor col;
					col.setNamedColor( oc[ 0 ] );
					col.setAlpha( oc[ 1 ].toInt() );
					myOutlinePen.setColor( col );
				}
			}
		}
		if ( desc.count() >= 12 ) {
			arrowType = desc[9].toInt();
			arrowSize = desc[10].toInt();
			arrowPos = desc[11].toInt();
		}
		sl.takeFirst();
	}	
	
	QImage image( 10, 10, QImage::Format_ARGB32_Premultiplied );
	QPainter painter;
	painter.begin( &image );
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
	if ( w > MAXW ) {
		x -= (w - MAXW) / 2.0;
		w = MAXW;
	}
	if ( h > MAXH ) {
		y -= (h - MAXH) / 2.0;
		h = MAXH;
	}
	
	image = QImage( w, h, QImage::Format_ARGB32_Premultiplied );
	QColor bk = myPen.color();
	bk.setAlpha(0);
	image.fill( bk );
	painter.begin( &image );
	painter.setRenderHints( QPainter::Antialiasing );
	if ( backgroundColor.alpha() > 0 ) {
		painter.setPen( QColor(0,0,0,0) );
		painter.setBrush( backgroundColor );
		painter.drawRect( 1, 1, w - 2, h - 2 );
	}	
	painter.setPen( myPen );
	painter.setBrush( myBrush );
	painter.setFont( myFont );

	QPainterPath myPath;
	
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
		myPath.addText( point, myFont, sl[i] );
		y += metrics.lineSpacing();
	}
	
	if ( outline ) {
		painter.setPen( myOutlinePen );
		painter.setBrush(Qt::NoBrush);
		painter.drawPath( myPath );
	}
	painter.setPen( Qt::NoPen );
	painter.setBrush(myBrush);
	painter.drawPath( myPath );
	
	painter.end();
	
	double ra = MAXW / MAXH;
	double ar = (double)iwidth / (double)iheight;
	double sw, sh;
	if (ar < ra) {
		sh = image.height() * iheight / MAXH;
		sw = image.width() * iheight / MAXH;
	}
	else {
		sw = image.width() * iwidth / MAXW;
		sh = image.height() * iwidth / MAXW;
	}
	
	return new QImage(image.scaled( sw, sh, Qt::KeepAspectRatio, Qt::SmoothTransformation ));
}
