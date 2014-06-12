#ifndef VIDEOWIDGET_H
#define VIDEOWIDGET_H

#define GL_GLEXT_PROTOTYPES

#include "engine/frame.h"

#include <QGLFramebufferObject>

#include <QGLWidget>
#include <QFile>
#include <QTimer>
#include <QWheelEvent>
#include <QSlider>



class VideoWidget : public QGLWidget
{
	Q_OBJECT
public:
	VideoWidget( QWidget *parent=0 );
	~VideoWidget();

	QGLWidget* getSharedContext();
	QImage getThumb( int h, double *pts, bool outPts );

public slots:
	void showFrame( Frame *frame );
	void shot();
	void setBlackBackground( bool b );

protected :
	void initializeGL();
	void resizeGL( int, int );
	void paintGL();

	void wheelEvent( QWheelEvent * event );
	void mouseMoveEvent( QMouseEvent * event );
	void mousePressEvent( QMouseEvent * event );

private:
	QImage lastImage();
	
	QGLWidget *hidden, *thumb, *fences;
	GLuint boardbg, blackbg, background;
	double lastFrameRatio;
	Frame *lastFrame;

	QSlider *valSlider;

signals:
	void playPause();
	void wheelSeek( int );
	void move( int, int );
	void newSharedContext(QGLWidget*);
	void newThumbContext(QGLWidget*);
	void newFencesContext(QGLWidget*);
	void frameShown( Frame* );

	void toggleFullscreen();

	void valueChanged( int );
};
#endif // VIDEOWIDGET_H
