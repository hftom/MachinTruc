#include <stdio.h>
#include <sys/time.h>
#include <math.h>

#include <QThread>
#include <QFile>

#include "videoout/videowidget.h"



VideoWidget::VideoWidget( QWidget *parent ) : QGLWidget( QGLFormat(QGL::SampleBuffers), parent ),
	lastFrameRatio( 16./9. ),
	lastFrame( NULL )
{
	setAttribute( Qt::WA_OpaquePaintEvent );
	setAutoFillBackground( false );
	
	connect( &osdMessage, SIGNAL(update()), this, SLOT(update()) );
	connect( &osdTimer, SIGNAL(update()), this, SLOT(update()) );
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

	GLfloat left, right, top, bottom;
	GLfloat war = (GLfloat)w / (GLfloat)h;

	if ( war < lastFrameRatio ) {
		left = -1.0;
		right = 1.0;
		top = -war / lastFrameRatio;
		bottom = war / lastFrameRatio;
	}
	else {
		top = -1.0;
		bottom = 1.0;
		left = -lastFrameRatio / war;
		right = lastFrameRatio / war;
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



void VideoWidget::paintEvent( QPaintEvent *event )
{
	makeCurrent();

	openglDraw();
	
	QPainter painter( this );
	painter.setRenderHint( QPainter::Antialiasing );
	osdMessage.draw( &painter, width(), height() );
	osdTimer.draw( &painter, width(), height() );
}



void VideoWidget::showFrame( Frame *frame )
{
	lastFrameRatio = frame->glSAR * (double)frame->glWidth / (double)frame->glHeight;
	lastFrame = frame;
	emit frameShown( frame );
	osdTimer.stop();
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



void VideoWidget::mouseMoveEvent( QMouseEvent * event )
{
	Q_UNUSED( event );
	//if ( event->button() == Qt::LeftButton )
		//emit move( event->x(), event->y() );
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



void VideoWidget::showOSDTimer()
{
	osdTimer.start();
}

