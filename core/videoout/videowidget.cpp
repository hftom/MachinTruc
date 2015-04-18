#include <stdio.h>
#include <sys/time.h>
#include <math.h>

#include <Eigen/Core>
#include <Eigen/Geometry>

#include <QThread>
#include <QFile>

#include "videoout/videowidget.h"

using namespace Eigen;



VideoWidget::VideoWidget( QWidget *parent ) : QGLWidget( QGLFormat(QGL::SampleBuffers), parent ),
	lastFrameRatio( 16./9. ),
	lastFrame( NULL )
{
	setAttribute( Qt::WA_OpaquePaintEvent );
	setAutoFillBackground( false );
	
	connect( &osdMessage, SIGNAL(update()), this, SLOT(update()) );
	connect( &osdTimer, SIGNAL(update()), this, SLOT(update()) );
	
	whiteDash.setStyle( Qt::CustomDashLine );
	QVector<qreal> dashes;
	dashes << 5 << 5;
	whiteDash.setDashPattern( dashes );
	whiteDash.setColor( "white" );
	
	setMouseTracking( true );
}



VideoWidget::~VideoWidget()
{
}



void VideoWidget::initializeGL()
{
	glEnable(GL_MULTISAMPLE);

	QGLWidget *hidden = new QGLWidget( NULL, this );
	if ( hidden ) {
		hidden->hide();
		emit newSharedContext( hidden );
	}
	
	QGLWidget *thumb = new QGLWidget();
	if ( thumb ) {
		thumb->hide();
		emit newThumbContext( thumb );
	}
	
	QGLWidget *fences = new QGLWidget( NULL, this );
	if ( fences ) {
		fences->hide();
		emit newFencesContext( fences );
	}
}



void VideoWidget::resizeGL( int width, int height )
{
	glViewport( 0, 0, width, height );
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0, width, 0.0, height, -1.0, 1.0);
	glMatrixMode(GL_MODELVIEW);
}



void VideoWidget::openglDraw()
{
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	
	glClearColor( 0.2f, 0.2f, 0.2f, 0.0f );
	glEnable( GL_TEXTURE_2D );
	
	int w = width();
	int h = height();

	glViewport( 0, 0, w, h );
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	glOrtho( 0.0, w, 0.0, h, -1.0, 1.0 );
	glMatrixMode( GL_MODELVIEW );

	GLfloat war = (GLfloat)w / (GLfloat)h;

	if ( war < lastFrameRatio ) {
		left = -1.0;
		right = 1.0;
		bottom = war / lastFrameRatio;
		top = -bottom;
	}
	else {
		top = -1.0;
		bottom = 1.0;
		right = lastFrameRatio / war;
		left = -right;
	}

	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	glPushMatrix();
	glTranslatef( w / 2.0, h / 2.0, 0 );
	glScalef( w / 2.0, h / 2.0, 1.0 );
	
	if ( lastFrame && lastFrame->fbo() ) {
		glBindTexture( GL_TEXTURE_2D, lastFrame->fbo()->texture() );
		glBegin( GL_QUADS );
			glTexCoord2f( 0, 0 );
			glVertex3f( left, top, 0 );
			glTexCoord2f( 0, 1 );
			glVertex3f( left, bottom, 0 );
			glTexCoord2f( 1, 1 );
			glVertex3f( right, bottom, 0 );
			glTexCoord2f( 1, 0 );
			glVertex3f( right, top, 0 );
		glEnd();
	}

	glPopMatrix();
	
	glDisable( GL_TEXTURE_2D );
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}



void VideoWidget::drawOVD( QPainter *painter, bool nonSquare )
{
	for ( int i = 0; i < lastFrame->sample->frames.count(); ++i ) {
		Frame *f = lastFrame->sample->frames[i]->frame ;
		if ( f && f->glOVD ) {
			Vector2d topleft( f->glOVDRect.x(), f->glOVDRect.y() );
			Vector2d topright( f->glOVDRect.x() + f->glOVDRect.width(), f->glOVDRect.y() );
			Vector2d bottomleft( f->glOVDRect.x(), f->glOVDRect.y() + f->glOVDRect.height() );
			Vector2d bottomright( f->glOVDRect.x() + f->glOVDRect.width(), f->glOVDRect.y() + f->glOVDRect.height() );
				
			for ( int j = 0; j < f->glOVDTransformList.count(); ++j ) {
				FilterTransform ft = f->glOVDTransformList.at( j );
				switch ( ft.transformType ) {
					case FilterTransform::SCALE: {
						topleft = Scaling( ft.v1, ft.v2 ) * topleft;
						topright = Scaling( ft.v1, ft.v2 ) * topright;
						bottomleft = Scaling( ft.v1, ft.v2 ) * bottomleft;
						bottomright = Scaling( ft.v1, ft.v2 ) * bottomright;
						break;
					}
					case FilterTransform::TRANSLATE: {
						Translation<double,2> trans( Vector2d(ft.v1, ft.v2) );
						topleft = trans * topleft;
						topright = trans * topright;
						bottomleft = trans * bottomleft;
						bottomright = trans * bottomright;
						break;
					}
					case FilterTransform::ROTATE: {
						if ( nonSquare ) {
							topleft = Scaling( lastFrame->glSAR, 1.0 ) * topleft;
							topright = Scaling( lastFrame->glSAR, 1.0 ) * topright;
							bottomleft = Scaling( lastFrame->glSAR, 1.0 ) * bottomleft;
							bottomright = Scaling( lastFrame->glSAR, 1.0 ) * bottomright;
						}
				
						Rotation2D<double> rot( -ft.v1 );
						topleft = rot * topleft;
						topright = rot * topright;
						bottomleft = rot * bottomleft;
						bottomright = rot * bottomright;
						
						if ( nonSquare ) {
							topleft = Scaling( 1.0 / lastFrame->glSAR, 1.0 ) * topleft;
							topright = Scaling( 1.0 / lastFrame->glSAR, 1.0 ) * topright;
							bottomleft = Scaling( 1.0 / lastFrame->glSAR, 1.0 ) * bottomleft;
							bottomright = Scaling( 1.0 / lastFrame->glSAR, 1.0 ) * bottomright;
						}
						break;
					}
				}
			}

			if ( nonSquare ) {
				topleft = Scaling( lastFrame->glSAR, 1.0 ) * topleft;
				topright = Scaling( lastFrame->glSAR, 1.0 ) * topright;
				bottomleft = Scaling( lastFrame->glSAR, 1.0 ) * bottomleft;
				bottomright = Scaling( lastFrame->glSAR, 1.0 ) * bottomright;
			}
			
			double sc = (double)width() / (lastFrame->glSAR * lastFrame->glWidth) * right;
			topleft = Scaling( sc, sc ) * topleft;
			topright = Scaling( sc, sc ) * topright;
			bottomleft = Scaling( sc, sc ) * bottomleft;
			bottomright = Scaling( sc, sc ) * bottomright;
			
			Translation<double,2> trans( Vector2d( (double)width() / 2.0, (double)height() / 2.0 ) );
			topleft = trans * topleft;
			topright = trans * topright;
			bottomleft = trans * bottomleft;
			bottomright = trans * bottomright;
			
			ovdPoints[0] = ovdPoints[4] = QPointF( topleft.x(), topleft.y() );
			ovdPoints[1] = QPointF( topright.x(), topright.y() );
			ovdPoints[2] = QPointF( bottomright.x(), bottomright.y() );
			ovdPoints[3] = QPointF( bottomleft.x(), bottomleft.y() );
			
			painter->save();
			/*painter->translate( (double)width() / 2.0, (double)height() / 2.0 );
			double sc = (double)width() / (lastFrame->glSAR * lastFrame->glWidth) * right;
			painter->scale( sc, sc );*/
			painter->setPen( "black" );
			painter->drawPolyline( ovdPoints, 5 );
			painter->setPen( whiteDash );
			painter->drawPolyline( ovdPoints, 5 );
			painter->restore();
			break;
		}
	}
}



void VideoWidget::paintEvent( QPaintEvent *event )
{
	makeCurrent();

	openglDraw();
	
	QPainter painter( this );
	painter.setRenderHint( QPainter::Antialiasing );
	
	if ( lastFrame && lastFrame->sample ) {
		drawOVD( &painter, qAbs( lastFrame->glSAR - 1.0 ) > 1e-3 );
	}
	
	osdMessage.draw( &painter, width(), height() );
	osdTimer.draw( &painter, width(), height() );
}



void VideoWidget::showFrame( Frame *frame )
{
	lastFrameRatio = frame->glSAR * (double)frame->glWidth / (double)frame->glHeight;
	lastFrame = frame;
	emit frameShown( frame );
	osdTimer.disable();
	update();
}



void VideoWidget::clear()
{
	lastFrame = NULL;
	lastFrameRatio = 16. / 9.;
	update();
}



QImage VideoWidget::lastImage()
{
	if ( !lastFrame )
		return QImage();
	
	makeCurrent();
	
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glEnable( GL_TEXTURE_2D );
	
	int h = lastFrame->glHeight;
	int w = lastFrame->glWidth * lastFrame->glSAR;
	QGLFramebufferObject fb( w, h );
	fb.bind();
	glViewport( 0, 0, w, h );
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	glOrtho( 0.0, w, 0.0, h, -1.0, 1.0 );
	glMatrixMode( GL_MODELVIEW );
	glActiveTexture( GL_TEXTURE0 );
	glBindTexture( GL_TEXTURE_2D, lastFrame->fbo()->texture() );
	glBegin( GL_QUADS );
		glTexCoord2f( 0, 0 );
		glVertex3f( 0, 0, 0.);
		glTexCoord2f( 0, 1 );
		glVertex3f( 0, h, 0.);
		glTexCoord2f( 1, 1 );
		glVertex3f( w, h, 0.);
		glTexCoord2f( 1, 0 );
		glVertex3f( w, 0, 0.);
	glEnd();
	fb.release();
	
	glDisable( GL_TEXTURE_2D );
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

	return fb.toImage();
}



void VideoWidget::shot()
{
	QImage img = lastImage();
	if ( img.isNull() )
		return;
	int i=0;
	while ( QFile::exists( QString("shot%1.png").arg( i ) ) )
		++i;
	img.save(QString("shot%1.png").arg( i ));
}



void VideoWidget::wheelEvent( QWheelEvent * event )
{
	int d = 1;

	if ( event->modifiers() & Qt::ControlModifier )
		d *= 10;
	if ( event->delta() < 0 )
		d *= -1;

	event->accept();
	emit wheelSeek( d );
}


#define CURSORDISTANCE 6
void VideoWidget::mouseMoveEvent( QMouseEvent * event )
{
	if ( lastFrame && lastFrame->sample ) {
		for ( int i = 0; i < lastFrame->sample->frames.count(); ++i ) {
			Frame *f = lastFrame->sample->frames[i]->frame ;
			if ( f && f->glOVD ) {
				if ( (ovdPoints[0] - event->pos()).manhattanLength() < CURSORDISTANCE )
					setCursor( QCursor(Qt::SizeFDiagCursor) );
				else if ( (ovdPoints[2] - event->pos()).manhattanLength() < CURSORDISTANCE )
					setCursor( QCursor(Qt::SizeFDiagCursor) );
				else if ( (ovdPoints[1] - event->pos()).manhattanLength() < CURSORDISTANCE )
					setCursor( QCursor(Qt::SizeBDiagCursor) );
				else if ( (ovdPoints[3] - event->pos()).manhattanLength() < CURSORDISTANCE )
					setCursor( QCursor(Qt::SizeBDiagCursor) );
				else
					setCursor( QCursor(Qt::ArrowCursor) );
				break;
			}
		}
	}
}



void VideoWidget::mousePressEvent( QMouseEvent * event )
{
	if ( event->button() == Qt::LeftButton ) {
		emit playPause();
	}
}



void VideoWidget::showOSDMessage( const QString &text, int duration )
{
	osdMessage.setMessage( text, duration );
}



void VideoWidget::showOSDTimer( bool b )
{
	if ( b )
		osdTimer.start();
	else
		osdTimer.stop();
}

