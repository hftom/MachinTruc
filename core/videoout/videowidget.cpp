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

static const char* vertexShader =
"#version 330\n"
"uniform vec2 factor;\n"
"const vec2 quadVertices[4] = vec2[4]( vec2( -1.0, -1.0), vec2( 1.0, -1.0), vec2( -1.0, 1.0), vec2( 1.0, 1.0));\n"
"const vec2 coords[4] = vec2[4]( vec2( 0.0, 0.0), vec2( 1.0, 0.0), vec2( 0.0, 1.0), vec2( 1.0, 1.0));\n"
"out vec2 uv;\n"
"void main( void ) {\n"
"      gl_Position = vec4(quadVertices[gl_VertexID] * factor, 0.0, 1.0);"
"      uv = coords[gl_VertexID];\n"
"}\n";

static const char* fragmentShader =
"#version 330\n"
"in vec2 uv;\n"
"out vec4 fragColor;\n"
"uniform sampler2D sampler;\n"
"void main( void ) {\n"
"      fragColor = texture(sampler, uv);\n"
"}\n";



VideoWidget::VideoWidget( QWidget *parent ) : QOpenGLWidget( parent ),
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
	GLSharedContext *hidden = new GLSharedContext( this->context() );
	if ( hidden ) {
		emit newSharedContext( hidden );
	}

	GLSharedContext *thumb = new GLSharedContext( this->context() );
	if ( thumb ) {
		emit newThumbContext( thumb );
	}

	GLSharedContext *fences = new GLSharedContext( this->context() );
	if ( fences ) {
		emit newFencesContext( fences );
	}

	makeCurrent();

	uint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glEnable(GL_MULTISAMPLE);

	bool result = shader.addShaderFromSourceCode( QOpenGLShader::Vertex, vertexShader );
	if ( !result ) {
		qWarning() << shader.log();
	}
	result = shader.addShaderFromSourceCode( QOpenGLShader::Fragment, fragmentShader );
	if ( !result ) {
		qWarning() << shader.log();
	}
	result = shader.link();
	if ( !result ) {
		qWarning() << "Could not link shader program:" << shader.log();
	}
	factorLocation = shader.uniformLocation("factor");
}



void VideoWidget::resizeGL( int width, int height )
{
	glViewport( 0, 0, width, height );
	GLfloat war = (GLfloat)width / (GLfloat)height;
	if ( war < lastFrameRatio ) {
		fx = 1.0;
		fy = war / lastFrameRatio;
	}
	else {
		fy = 1.0;
		fx = lastFrameRatio / war;
	}

	shader.bind();
	shader.setUniformValue(factorLocation, fx, fy);
	shader.release();
}



void VideoWidget::paintGL()
{
	glClearColor( 0.2f, 0.2f, 0.2f, 0.0f );
	glEnable( GL_TEXTURE_2D );
	if ( lastFrame && lastFrame->fbo() )
		glBindTexture( GL_TEXTURE_2D, lastFrame->fbo()->texture() );
	shader.bind();
	glDrawArrays( GL_TRIANGLE_STRIP, 0, 4 );
	shader.release();
	glDisable( GL_TEXTURE_2D );

#if !defined(Q_OS_MAC)
	QPainter painter( this );
	painter.setRenderHint( QPainter::Antialiasing );

	if ( !playing && lastFrame && lastFrame->sample ) {
		drawOVD( &painter, qAbs( lastFrame->glSAR - 1.0 ) > 1e-3 );
	}

	osdMessage.draw( &painter, width(), height() );
	osdTimer.draw( &painter, width(), height() );
#endif
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

	glEnable( GL_TEXTURE_2D );

	int h = lastFrame->glHeight;
	int w = lastFrame->glWidth * lastFrame->glSAR;
	QOpenGLFramebufferObject fb( w, h );
	fb.bind();

	glViewport( 0, 0, w, h );
	glActiveTexture( GL_TEXTURE0 );
	glBindTexture( GL_TEXTURE_2D, lastFrame->fbo()->texture() );
	shader.bind();
	shader.setUniformValue(factorLocation, 1.0, 1.0);
	glDrawArrays( GL_TRIANGLE_STRIP, 0, 4 );
	shader.setUniformValue(factorLocation, fx, fy);
	shader.release();
	fb.release();

	glDisable( GL_TEXTURE_2D );

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
#if defined(Q_OS_MAC)
	return;
#endif

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
#if defined(Q_OS_MAC)
	return;
#endif

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

				break;
			}
		}
	}

	ovdTarget = target;
	if ( ovdTarget == 0 && cursor().shape() != Qt::ArrowCursor )
		setCursor( QCursor(Qt::ArrowCursor) );
}



void VideoWidget::controlKeyPressed( bool down )
{
#if defined(Q_OS_MAC)
	return;
#endif

	QMouseEvent event( QEvent::MouseMove, mapFromGlobal( QCursor::pos() ), Qt::NoButton, Qt::NoButton, down ? Qt::ControlModifier : Qt::NoModifier );
	mouseMoveEvent( &event );
}



void VideoWidget::mousePressEvent( QMouseEvent * event )
{
#if defined(Q_OS_MAC)
	return;
#endif

	if ( event->button() & Qt::LeftButton ) {
		leftButtonPressed = true;
		mousePressedPoint = QPointF( event->pos() );
		ovdPointsMousePressed = ovdPoints;
		ovdTransformListMousePressed = ovdTransformList;
	}
}



void VideoWidget::mouseReleaseEvent( QMouseEvent * event )
{
#if defined(Q_OS_MAC)
	return;
#endif

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
			double sc = (double)width() / (lastFrame->glSAR * lastFrame->glWidth) * fx;
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
	double sc = (double)width() / (lastFrame->glSAR * lastFrame->glWidth) * fx;
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
