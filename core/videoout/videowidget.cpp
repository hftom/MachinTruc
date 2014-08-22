#include <stdio.h>
#include <sys/time.h>
#include <math.h>

#include <QThread>
#include <QFile>

#include "videoout/videowidget.h"

#define BOARDWIDTH 32
#define HICOLOR 153
#define LOCOLOR 102



VideoWidget::VideoWidget( QWidget *parent ) : QGLWidget( parent )
{
	setAutoFillBackground( false );
	lastFrameRatio = 16./9.;
	hidden = thumb = fences = NULL;
	lastFrame = NULL;
}



VideoWidget::~VideoWidget()
{
}



void VideoWidget::initializeGL()
{
	glClearColor( 0.2f, 0.2f, 0.2f, 0.0f );
	glClearDepth( 1.0f );
	glDepthFunc( GL_LEQUAL );
	glDisable( GL_DEPTH_TEST );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
	glDisable( GL_BLEND );
	glShadeModel( GL_SMOOTH );
	glEnable( GL_TEXTURE_2D );
	glHint( GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST );

	uint8_t img[ BOARDWIDTH * BOARDWIDTH ];
	int i = 0, loop = 0;
	bool color = true;
	while ( i < BOARDWIDTH * BOARDWIDTH ) {
		memset( img + i, (color) ? HICOLOR : LOCOLOR, BOARDWIDTH / 2 );
		if ( ++loop >= BOARDWIDTH )
			loop = 0;
		if ( loop )
			color = !color;
		i += BOARDWIDTH / 2;
	}
	boardbg = bindTexture( QImage(BOARDWIDTH, BOARDWIDTH, QImage::Format_Indexed8), GL_TEXTURE_2D, GL_LUMINANCE );
	glTexImage2D( GL_TEXTURE_2D, 0, GL_LUMINANCE, BOARDWIDTH, BOARDWIDTH, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, img );
	
	memset( img, 0, BOARDWIDTH * BOARDWIDTH );
	blackbg = bindTexture( QImage(BOARDWIDTH, BOARDWIDTH, QImage::Format_Indexed8), GL_TEXTURE_2D, GL_LUMINANCE );
	glTexImage2D( GL_TEXTURE_2D, 0, GL_LUMINANCE, BOARDWIDTH, BOARDWIDTH, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, img );
	
	background = boardbg;

#define GL_GPU_MEM_INFO_TOTAL_AVAILABLE_MEM_NVX 0x9048
#define GL_GPU_MEM_INFO_CURRENT_AVAILABLE_MEM_NVX 0x9049
	GLint total_mem_kb = 0;
	glGetIntegerv(GL_GPU_MEM_INFO_TOTAL_AVAILABLE_MEM_NVX, &total_mem_kb);
	GLint cur_avail_mem_kb = 0;
	glGetIntegerv(GL_GPU_MEM_INFO_CURRENT_AVAILABLE_MEM_NVX, &cur_avail_mem_kb);
	printf("Total GPU mem = %d MB, Available = %d MB\n", total_mem_kb/1024, cur_avail_mem_kb/1024);

	hidden = new QGLWidget( NULL, this );
	if ( hidden ) {
		hidden->hide();
		emit newSharedContext( hidden );
	}
	
	thumb = new QGLWidget();
	if ( thumb ) {
		thumb->hide();
		emit newThumbContext( thumb );
	}
	
	fences = new QGLWidget( NULL, this );
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
	updateGL();
}



void VideoWidget::paintGL()
{
	int w = width();
	int h = height();

	glViewport( 0, 0, w, h );
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	glOrtho( 0.0, w, 0.0, h, -1.0, 1.0 );
	glMatrixMode( GL_MODELVIEW );

	GLfloat left, right, top, bottom;
	GLfloat war = (GLfloat)w / (GLfloat)h;
	GLfloat u, v;

	if ( war < lastFrameRatio ) {
		left = -1.0;
		right = 1.0;
		top = -war / lastFrameRatio;
		bottom = war / lastFrameRatio;
		u = (GLfloat)w / 32.0;
		v = u / lastFrameRatio;
	}
	else {
		top = -1.0;
		bottom = 1.0;
		left = -lastFrameRatio / war;
		right = lastFrameRatio / war;
		v = (GLfloat)h / 32.0;
		u = v * lastFrameRatio;
	}

	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	glPushMatrix();

	glTranslatef( w / 2.0, h / 2.0, 0 );
	glScalef( w / 2.0, h / 2.0, 1.0 );

	glActiveTexture( GL_TEXTURE0 );
	glBindTexture( GL_TEXTURE_2D, background );
	glBegin( GL_QUADS );
		glTexCoord2f( 0, 0 );
		glVertex3f( left, top, 0.);
		glTexCoord2f( 0, v );
		glVertex3f( left, bottom, 0.);
		glTexCoord2f( u, v );
		glVertex3f( right, bottom, 0.);
		glTexCoord2f( u, 0 );
		glVertex3f( right, top, 0.);
	glEnd();
	
	if ( lastFrame ) {
		glEnable( GL_BLEND );
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
		glDisable( GL_BLEND );
	}

	glPopMatrix();
}



void VideoWidget::showFrame( Frame *frame )
{
	lastFrameRatio = frame->glSAR * (double)frame->glWidth / (double)frame->glHeight;
	if ( lastFrame != frame ) {
		lastFrame = frame;
		emit frameShown( frame );
	}
	updateGL();
}



QImage VideoWidget::lastImage()
{
	if ( !lastFrame )
		return QImage();
	
	makeCurrent();
	
	int h = lastFrame->glHeight;
	int w = lastFrame->glWidth * lastFrame->glSAR;
	QGLFramebufferObject fb( w, h );
	fb.bind();
	glClearColor( 0, 0, 0, 0 );
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	glViewport( 0, 0, w, h );
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	glOrtho( 0.0, w, 0.0, h, -1.0, 1.0 );
	glMatrixMode( GL_MODELVIEW );
	glActiveTexture( GL_TEXTURE0 );
	/*glBindTexture( GL_TEXTURE_2D, background );
	glBegin( GL_QUADS );
		glTexCoord2f( 0, 0 );
		glVertex3f( 0, h, 0.);
		glTexCoord2f( 0, h/BOARDWIDTH );
		glVertex3f( 0, 0, 0.);
		glTexCoord2f( w/BOARDWIDTH, h/BOARDWIDTH );
		glVertex3f( w, 0, 0.);
		glTexCoord2f( w/BOARDWIDTH, 0 );
		glVertex3f( w, h, 0.);
	glEnd();*/
	glBindTexture( GL_TEXTURE_2D, lastFrame->fbo()->texture() );
	glEnable( GL_BLEND );
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
	glDisable( GL_BLEND );
	glClearColor( 0.2f, 0.2f, 0.2f, 0.0f );
	fb.release();

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



void VideoWidget::setBlackBackground( bool b )
{
	background = b ? blackbg : boardbg;
	updateGL();
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
	//if ( event->button() == Qt::LeftButton )
		//emit move( event->x(), event->y() );
}



void VideoWidget::mousePressEvent( QMouseEvent * event )
{
	if ( event->button() == Qt::LeftButton ) {
		emit playPause();
	}
}

