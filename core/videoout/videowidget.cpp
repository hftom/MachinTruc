#include <stdio.h>
#include <sys/time.h>
#include <math.h>

#include <QTransform>
#include <QThread>
#include <QFile>

#include "videoout/videowidget.h"

#define OVDTOPLEFT 1
#define OVDTOPRIGHT 2
#define OVDBOTTOMRIGHT 3
#define OVDBOTTOMLEFT 4
#define OVDZOOMCENTER 5
#define OVDINSIDE 6
#define OVDOUTSIDE 7



VideoWidget::VideoWidget( QWidget *parent ) : QGLWidget( QGLFormat(QGL::SampleBuffers), parent ),
	lastFrameRatio( 16./9. ),
	lastFrame( NULL ),
	leftButtonPressed( false ),
	ovdTarget( 0 ),
	playing( false )
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



void VideoWidget::paintEvent( QPaintEvent *event )
{
	makeCurrent();

	openglDraw();
	
	QPainter painter( this );
	painter.setRenderHint( QPainter::Antialiasing );
	
	if ( !playing && lastFrame && lastFrame->sample ) {
		drawOVD( &painter, qAbs( lastFrame->glSAR - 1.0 ) > 1e-3 );
	}
	
	osdMessage.draw( &painter, width(), height() );
	osdTimer.draw( &painter, width(), height() );
}



void VideoWidget::showFrame( Frame *frame )
{
	lastFrameRatio = frame->glSAR * (double)frame->glWidth / (double)frame->glHeight;
	lastFrame = frame;
	osdTimer.disable();
	update();
	emit frameShown( frame );
}



void VideoWidget::clear()
{
	lastFrame = NULL;
	lastFrameRatio = 16. / 9.;
	update();
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
	playing = b;
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


#define CURSORDISTANCE 12
void VideoWidget::mouseMoveEvent( QMouseEvent * event )
{
	int target = 0;
	if ( !playing && lastFrame && lastFrame->sample ) {
		for ( int i = 0; i < lastFrame->sample->frames.count(); ++i ) {
			Frame *f = lastFrame->sample->frames[i]->frame ;
			if ( f && f->glOVD ) {
				if ( leftButtonPressed ) {
					if ( ovdTarget > 0 )
						ovdUpdate( lastFrame->sample->frames[i], QPointF( event->pos() ) );
					return;
				}
				if ( event->modifiers() & Qt::ControlModifier ) {
					bool inside = cursorInside( event->pos() );
					if ( inside ) {
						if ( f->glOVD & FilterTransform::SCALE ) {
							setCursor( QCursor(Qt::SizeAllCursor) );
							target = OVDZOOMCENTER;
						}
					}
					else {
						setCursor( QCursor(Qt::PointingHandCursor) );
						target = OVDOUTSIDE;
					}					
				}
				else {
					if ( f->glOVD & FilterTransform::SCALE ) {
						if ( qAbs((ovdPoints[0] - event->pos()).manhattanLength()) < CURSORDISTANCE ) {
							setCursor( QCursor(Qt::SizeAllCursor) );
							target = OVDTOPLEFT;
						}
						else if ( qAbs((ovdPoints[2] - event->pos()).manhattanLength()) < CURSORDISTANCE ) {
							setCursor( QCursor(Qt::SizeAllCursor) );
							target = OVDBOTTOMRIGHT;
						}
						else if ( qAbs((ovdPoints[1] - event->pos()).manhattanLength()) < CURSORDISTANCE ) {
							setCursor( QCursor(Qt::SizeAllCursor) );
							target = OVDTOPRIGHT;
						}
						else if ( qAbs((ovdPoints[3] - event->pos()).manhattanLength()) < CURSORDISTANCE ) {
							setCursor( QCursor(Qt::SizeAllCursor) );
							target = OVDBOTTOMLEFT;
						}
					}
					if ( !target && cursorInside( event->pos() ) ) {
						setCursor( QCursor(Qt::PointingHandCursor) );
						target = OVDINSIDE;
					}
				}
			}
		}
	}
	
	ovdTarget = target;
	if ( ovdTarget == 0 && cursor().shape() != Qt::ArrowCursor )
		setCursor( QCursor(Qt::ArrowCursor) );
}



void VideoWidget::controlKeyPressed( bool down )
{
	QMouseEvent event( QEvent::MouseMove, mapFromGlobal( QCursor::pos() ), Qt::NoButton, Qt::NoButton, down ? Qt::ControlModifier : Qt::NoModifier );
	mouseMoveEvent( &event );
}



void VideoWidget::mousePressEvent( QMouseEvent * event )
{
	if ( event->button() & Qt::LeftButton ) {
		leftButtonPressed = true;
		mousePressedPoint = QPointF( event->pos() );
		ovdPointsMousePressed = ovdPoints;
		ovdTransformListMousePressed = ovdTransformList;
	}
}



void VideoWidget::mouseReleaseEvent( QMouseEvent * event )
{
	leftButtonPressed = false;
}



void VideoWidget::drawOVD( QPainter *painter, bool nonSquare )
{
	for ( int i = 0; i < lastFrame->sample->frames.count(); ++i ) {
		Frame *f = lastFrame->sample->frames[i]->frame ;
		if ( f && f->glOVD > 0 ) {
			ovdPoints = QPolygonF( f->glOVDRect );
			ovdTransformList = f->glOVDTransformList;
				
			for ( int j = 0; j < f->glOVDTransformList.count(); ++j ) {
				FilterTransform ft = f->glOVDTransformList.at( j );
				switch ( ft.transformType ) {
					case FilterTransform::SCALE:
					case FilterTransform::NERATIO: {
						ovdPoints = QTransform::fromScale( ft.v1, ft.v2 ).map( ovdPoints );
						break;
					}
					case FilterTransform::TRANSLATE: {
						ovdPoints = QTransform::fromTranslate( ft.v1, ft.v2 ).map( ovdPoints );
						break;
					}
					case FilterTransform::ROTATE: {
						if ( nonSquare )
							ovdPoints = QTransform::fromScale( lastFrame->glSAR, 1.0 ).map( ovdPoints );
						ovdPoints = QTransform().rotateRadians( -ft.v1 ).map( ovdPoints );
						if ( nonSquare )
							ovdPoints = QTransform::fromScale( 1.0 / lastFrame->glSAR, 1.0 ).map( ovdPoints );
						break;
					}
				}
			}

			if ( nonSquare )
				ovdPoints = QTransform::fromScale( lastFrame->glSAR, 1.0 ).map( ovdPoints );
			double sc = (double)width() / (lastFrame->glSAR * lastFrame->glWidth) * right;
			ovdPoints = QTransform::fromScale( sc, sc ).map( ovdPoints );
			ovdPoints = QTransform::fromTranslate( (double)width() / 2.0, (double)height() / 2.0 ).map( ovdPoints );

			painter->setPen( "black" );
			painter->drawPolygon( ovdPoints );
			painter->setPen( whiteDash );
			painter->drawPolygon( ovdPoints );
			break;
		}
	}
}



void VideoWidget::ovdUpdate( FrameSample *frameSample, QPointF cursorPos )
{	
	bool nonSquare = qAbs( lastFrame->glSAR - 1.0 ) > 1e-3;
	QPolygonF polygon = ovdPointsMousePressed;
	polygon.append( mousePressedPoint );
	polygon.append( cursorPos );

	polygon = QTransform::fromTranslate( -(double)width() / 2.0, -(double)height() / 2.0 ).map( polygon );
	double sc = (double)width() / (lastFrame->glSAR * lastFrame->glWidth) * right;
	polygon = QTransform::fromScale( 1.0 / sc, 1.0 / sc ).map( polygon );
	if ( nonSquare )
		polygon = QTransform::fromScale( 1.0 / lastFrame->glSAR, 1.0 ).map( polygon );	
	
	double t1 = 0.0, t2 = 0.0;
	double s1 = 1.0, s2 = 1.0;
	QPolygonF current = polygon;
	
	for ( int j = ovdTransformListMousePressed.count() - 1; j >= 0; --j ) {
		FilterTransform ft = ovdTransformListMousePressed.at( j );
		switch ( ft.transformType ) {
			case FilterTransform::NERATIO: {
				polygon = QTransform::fromScale( 1.0 / ft.v1, 1.0 / ft.v2 ).map( polygon );
				break;
			}
			case FilterTransform::SCALE: {
				polygon = QTransform::fromScale( 1.0 / ft.v1, 1.0 / ft.v2 ).map( polygon );
				if ( ovdTarget >= OVDTOPLEFT && ovdTarget <= OVDZOOMCENTER ) {
					current = polygon;
				}
				s1 = ft.v1;
				s2 = ft.v2;
				break;
			}
			case FilterTransform::TRANSLATE: {
				polygon = QTransform::fromTranslate( -ft.v1, -ft.v2 ).map( polygon );
				if ( ovdTarget == OVDINSIDE || ovdTarget == OVDOUTSIDE ) {
					current = polygon;
				}
				t1 = ft.v1;
				t2 = ft.v2;
				break;
			}
			case FilterTransform::ROTATE: {
				if ( nonSquare )
					polygon = QTransform::fromScale( 1.0 / lastFrame->glSAR, 1.0 ).map( polygon );
				polygon = QTransform().rotateRadians( ft.v1 ).map( polygon );
				if ( nonSquare )
					polygon = QTransform::fromScale( lastFrame->glSAR, 1.0 ).map( polygon );
				break;
			}
		}
	}

	QSharedPointer<GLFilter> filter;
	for ( int i = 0; i < frameSample->videoFilters.count(); ++i ) {
		if ( frameSample->videoFilters.at( i )->ovdEnabled() ) {
			filter = frameSample->videoFilters.at( i );
			break;
		}
	}
	
	if ( !filter.isNull() ) {
		if ( ovdTarget == OVDINSIDE || ovdTarget == OVDOUTSIDE ) {
			current[6] -= current[5];
			QList<OVDUpdateMessage> msg;
			msg.append( OVDUpdateMessage( filter, "translate", QPointF( t1, t2 ) + current[6] ) );
			emit ovdUpdateSignal( msg );
		}
		else if ( ovdTarget >= OVDTOPLEFT && ovdTarget <= OVDZOOMCENTER ) {
			double ow = current[1].x() - current[0].x();
			double oh = current[2].y() - current[1].y();
			double w, h;
			QList<OVDUpdateMessage> msg;
			switch ( ovdTarget ) {
				case OVDZOOMCENTER: {
					w = qMax( 0.0, ow + 2 * (current[6].x() - current[5].x()) );
					msg.append( OVDUpdateMessage( filter, "scale", QPointF( s1 * w * 100.0 / ow, 1.0 ) ) );
					break;
				}
				case OVDBOTTOMRIGHT: {
					w = qMax( 0.0, current[6].x() - current[0].x() );
					h = w * oh / ow;
					msg.append( OVDUpdateMessage( filter, "scale", QPointF( s1 * w * 100.0 / ow, 1.0 ) ) );
					msg.append( OVDUpdateMessage( filter, "translate", QPointF( t1, t2 ) + QPointF( (s1 * (w - ow)) / 2.0, (s2 * (h - oh)) / 2.0 ) ) );
					break;
				}
				case OVDBOTTOMLEFT: {
					w = qMax( 0.0, current[2].x() - current[6].x() );
					h = w * oh / ow;
					msg.append( OVDUpdateMessage( filter, "scale", QPointF( s1 * w * 100.0 / ow, 1.0 ) ) );
					msg.append( OVDUpdateMessage( filter, "translate", QPointF( t1, t2 ) - QPointF( (s1 * (w - ow)) / 2.0, (s2 * (oh - h)) / 2.0 ) ) );
					break;
				}
				case OVDTOPRIGHT: {
					w = qMax( 0.0, current[6].x() - current[0].x() );
					h = w * oh / ow;
					msg.append( OVDUpdateMessage( filter, "scale", QPointF( s1 * w * 100.0 / ow, 1.0 ) ) );
					msg.append( OVDUpdateMessage( filter, "translate", QPointF( t1, t2 ) + QPointF( (s1 * (w - ow)) / 2.0, (s2 * (oh - h)) / 2.0 ) ) );
					break;
				}
				case OVDTOPLEFT: {
					w = qMax( 0.0, current[2].x() - current[6].x() );
					h = w * oh / ow;
					msg.append( OVDUpdateMessage( filter, "scale", QPointF( s1 * w * 100.0 / ow, 1.0 ) ) );
					msg.append( OVDUpdateMessage( filter, "translate", QPointF( t1, t2 ) - QPointF( (s1 * (w - ow)) / 2.0, (s2 * (h - oh)) / 2.0 ) ) );
					break;
				}
			}
			emit ovdUpdateSignal( msg );
		}
	}
}
	


bool VideoWidget::cursorInside( QPointF cursorPos )
{
	if ( ovdPoints.count() < 4 )
		return false;

	double bax = ovdPoints[1].x() - ovdPoints[0].x();
	double bay = ovdPoints[1].y() - ovdPoints[0].y();
	double dax = ovdPoints[3].x() - ovdPoints[0].x();
	double day = ovdPoints[3].y() - ovdPoints[0].y();
	if ( (cursorPos.x() - ovdPoints[0].x() ) * bax + ( cursorPos.y() - ovdPoints[0].y() ) * bay < 0.0 )
		return false;
	if ( (cursorPos.x() - ovdPoints[1].x() ) * bax + ( cursorPos.y() - ovdPoints[1].y() ) * bay > 0.0 )
		return false;
	if ( (cursorPos.x() - ovdPoints[0].x() ) * dax + ( cursorPos.y() - ovdPoints[0].y() ) * day < 0.0 )
		return false;
	if ( (cursorPos.x() - ovdPoints[3].x() ) * dax + ( cursorPos.y() - ovdPoints[3].y() ) * day > 0.0 )
		return false;
	
	return true;
}
