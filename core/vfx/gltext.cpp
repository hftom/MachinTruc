#include <QPainter>

#include "engine/util.h"

#include "gltext.h"



GLText::GLText( QString id, QString name ) : GLFilter( id, name )
{
	editor = addParameter( "editor", tr("Editor:"), Parameter::PSTRING, "Arial,12,-1,5,50,0,0,0,0,0|30|0|0|#ffffff.255|#000000.0|1|0|#000000.255|0|50|50\nText", "", "", false );
	opacity = addParameter( "opacity", tr("Opacity:"), Parameter::PDOUBLE, 1.0, 0.0, 1.0, true );
	xOffset = addParameter( "xOffset", tr("X offset:"), Parameter::PINPUTDOUBLE, 0.0, -110.0, 110.0, true, "%" );
	yOffset = addParameter( "yOffset", tr("Y offset:"), Parameter::PINPUTDOUBLE, 0.0, -110.0, 110.0, true, "%" );
}



void GLText::ovdUpdate( QString type, QVariant val )
{
	if ( type == "translate" ) {
		ovdOffset = val;
	}
}



bool GLText::process( const QList<Effect*> &el, double pts, Frame *src, Profile *p )
{
	if (!ovdOffset.isNull()) {
		QPointF pos = ovdOffset.toPointF();
		if ( !xOffset->graph.keys.count() )
			xOffset->value = pos.x() * 100.0 / src->glWidth;
		if ( !yOffset->graph.keys.count() )
			yOffset->value = pos.y() * 100.0 / src->glHeight;
		
		ovdOffset = QVariant();
	}
	
	double xof = src->glWidth * getParamValue( xOffset, pts ).toDouble() / 100.0;
	double yof = src->glHeight * getParamValue( yOffset, pts ).toDouble() / 100.0;

	MyTextEffect *e = (MyTextEffect*)el[0];
	bool ok = e->set_float( "left", xof )
		&& e->set_float( "top", yof )
		&& e->set_float( "opacity", getParamValue( opacity, pts ).toDouble() );
		
	QString text = getParamValue( editor ).toString();
	if (text.contains("##")) {
		double secs = pts / MICROSECOND;
		int hour = secs / 3600;
		int min =(secs - (hour * 3600)) / 60;
		int sec = secs - (hour * 3600) - (min * 60);
		int img = qRound((secs - (hour * 3600) - (min * 60) - sec) * p->getVideoFrameRate());
		text.replace("##h##", QString("%1").arg(hour));
		text.replace("##hh##", QString("%1").arg(hour,2,10,QChar('0')));
		text.replace("##m##", QString("%1").arg(min));
		text.replace("##mm##", QString("%1").arg(min,2,10,QChar('0')));
		text.replace("##s##", QString("%1").arg(sec));
		text.replace("##ss##", QString("%1").arg(sec,2,10,QChar('0')));
		text.replace("##i##", QString("%1").arg(img));
		text.replace("##ii##", QString("%1").arg(img,2,10,QChar('0')));
	}
	e->setText(text, src->glWidth, src->glHeight);
		
	if ( ovdEnabled() ) {
		src->glOVD = FilterTransform::TRANSLATE;
		double w = e->getImageWidth(), h = e->getImageHeight();
		src->glOVDRect = QRectF( -w / 2.0, -h / 2.0, w, h );
		src->glOVDTransformList.append( FilterTransform( FilterTransform::TRANSLATE, xof, yof ) );
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
					myFont.setStyleStrategy( QFont::ForceOutline );
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
	if ( w > 1920 ) {
		x -= (w - 1920) / 2.0;
		w = 1920;
	}
	if ( h > 1080 ) {
		y -= (h - 1080) / 2.0;
		h = 1080;
	}
	
	QPointF polygon[7];
	arrowSize = h * arrowSize / 100.0;
	int n = 0;
	int leftOffset = 0, topOffset = 0;
	int wMargin = 0, hMargin = 0;
	if (arrowType) {
		switch (arrowType) {
			case 1: {
				leftOffset = arrowSize;
				wMargin = arrowSize;
				polygon[n].setX(1 + arrowSize);
				polygon[n++].setY(1);
				
				polygon[n].setY(qMax(1.0, qMin(h - 1.0, h * arrowPos / 100.0 - arrowSize / 2.0) ) );
				polygon[n++].setX(1 + arrowSize);
				polygon[n].setY(qMax(1.0, qMin(h - 1.0, h * arrowPos / 100.0) ) );
				polygon[n++].setX(1);
				polygon[n].setY(qMax(1.0, qMin(h - 1.0, h * arrowPos / 100.0 + arrowSize / 2.0) ) );
				polygon[n++].setX(1 + arrowSize);
				
				polygon[n].setX(1 + arrowSize);
				polygon[n++].setY(h - 1);
				polygon[n].setX(w - 1 + arrowSize);
				polygon[n++].setY(h - 1);
				polygon[n].setX(w - 1 + arrowSize);
				polygon[n++].setY(1);
				break;
			}
			case 2: {
				wMargin = arrowSize;
				polygon[n].setX(1);
				polygon[n++].setY(1);
				polygon[n].setX(1);
				polygon[n++].setY(h - 1);
				polygon[n].setX(w - 1);
				polygon[n++].setY(h - 1);
				
				polygon[n].setY(qMax(1.0, qMin(h - 1.0, h * arrowPos / 100.0 + arrowSize / 2.0) ) );
				polygon[n++].setX(w - 1);
				polygon[n].setY(qMax(1.0, qMin(h - 1.0, h * arrowPos / 100.0) ) );
				polygon[n++].setX(w - 1 + arrowSize);
				polygon[n].setY(qMax(1.0, qMin(h - 1.0, h * arrowPos / 100.0 - arrowSize / 2.0) ) );
				polygon[n++].setX(w - 1);
				
				polygon[n].setX(w - 1);
				polygon[n++].setY(1);
				break;
			}
			case 3: {
				topOffset = arrowSize;
				hMargin = arrowSize;
				polygon[n].setX(1);
				polygon[n++].setY(1 + arrowSize);
				polygon[n].setX(1);
				polygon[n++].setY(h - 1 + arrowSize);
				polygon[n].setX(w - 1);
				polygon[n++].setY(h - 1 + arrowSize);
				polygon[n].setX(w - 1);
				polygon[n++].setY(1 + arrowSize);
				
				polygon[n].setX(qMax(1.0, qMin(w - 1.0, w * arrowPos / 100.0 + arrowSize / 2.0) ) );
				polygon[n++].setY(1 + arrowSize);
				polygon[n].setX(qMax(1.0, qMin(w - 1.0, w * arrowPos / 100.0) ) );
				polygon[n++].setY(1);
				polygon[n].setX(qMax(1.0, qMin(w - 1.0, w * arrowPos / 100.0 - arrowSize / 2.0) ) );
				polygon[n++].setY(1 + arrowSize);
				break;
			}
			case 4: {
				hMargin = arrowSize;
				polygon[n].setX(1);
				polygon[n++].setY(1);
				polygon[n].setX(1);
				polygon[n++].setY(h - 1);
				
				polygon[n].setX(qMax(1.0, qMin(w - 1.0, w * arrowPos / 100.0 - arrowSize / 2.0) ) );
				polygon[n++].setY(h - 1);
				polygon[n].setX(qMax(1.0, qMin(w - 1.0, w * arrowPos / 100.0) ) );
				polygon[n++].setY(h - 1 + arrowSize);
				polygon[n].setX(qMax(1.0, qMin(w - 1.0, w * arrowPos / 100.0 + arrowSize / 2.0) ) );
				polygon[n++].setY(h - 1);
				
				polygon[n].setX(w - 1);
				polygon[n++].setY(h - 1);
				polygon[n].setX(w - 1);
				polygon[n++].setY(1);
				break;
			}
		}		
	}
	
	image = QImage( w + wMargin, h + hMargin, QImage::Format_ARGB32_Premultiplied );
	image.fill( QColor(0,0,0,0) );
	painter.begin( &image );
	painter.setRenderHints( QPainter::Antialiasing | QPainter::TextAntialiasing );
	if ( backgroundColor.alpha() > 0 ) {
		painter.setPen( QColor(0,0,0,0) );
		painter.setBrush( backgroundColor );
		if (arrowType) {
			painter.drawPolygon( polygon, 7 );
		}
		else {
			painter.drawRect( 1, 1, w - 2, h - 2 );
		}
	}	
	painter.setPen( myPen );
	painter.setBrush( myBrush );
	painter.setFont( myFont );

	for ( int i = 0; i < sl.count(); ++i ) {
		QPointF point( 0, y + topOffset + metrics.ascent() );
		switch ( align ) {
			case 2: {
				point.setX( leftOffset + (double)w / 2.0 - br[i].width() / 2.0 );
				break;
			}
			case 3: {
				point.setX( leftOffset + w - x - br[i].width() );
				break;
			}
			default: {
				point.setX( leftOffset + x );
				break;
			}
		}
		if ( outline ) {
			QPainterPath myPath;
			painter.setPen( myOutlinePen );
			myPath.addText( point, myFont, sl[i] );
			painter.drawPath( myPath );
		}
		painter.setPen( myPen );
		painter.drawText( point, sl[i] );

		y += metrics.lineSpacing();
	}
	painter.end();
	
	double ra = 1920.0 / 1080.0;
	double ar = (double)iwidth / (double)iheight;
	double sw, sh;
	if (ar < ra) {
		sh = image.height() * iheight / 1080.0;
		sw = image.width() * sh * ar;
	}
	else {
		sw = image.width() * iwidth / 1920.0;
		sh = image.height() * sw / ar;
	}
	
	return new QImage(image.scaled( sw, sh, Qt::KeepAspectRatio, Qt::SmoothTransformation ));
}
