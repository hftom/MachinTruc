#ifndef TOPWINDOW_H
#define TOPWINDOW_H

#include <QMainWindow>
#include <QSlider>
#include <QProgressDialog>

#include "ui_mainwindow.h"

#include "gui/projectclipspage.h"
#include "gui/fxpage.h"
#include "gui/fxsettingspage.h"

#include "engine/thumbnailer.h"
#include "engine/composer.h"
#include "videoout/videowidget.h"
#include "timeline/timeline.h"
#include "animation/animeditor.h"
#include "projectfile.h"



class MemoryFrame : public QFrame
{
	Q_OBJECT
public:
	MemoryFrame( QWidget *parent, MemChunk *c ) : QFrame( parent ), chunk(c) {
		setFixedSize( 128, 10 );
		setFrameStyle( QFrame::NoFrame );
	}
	virtual void paintEvent ( QPaintEvent* ) {
		int h = height();
		QPainter p;
		p.begin( this );
		for ( int i = 0; i < 128; ++i ) {
			if ( chunk->used[i] ) {
				if ( chunk->sliceSize == 1048576 )
					p.setPen( QColor("orange") );
				else if ( chunk->sliceSize == 102400 )
					p.setPen( QColor("cyan") );
				else
					p.setPen( QColor("white") );
			}
			else
				p.setPen( QColor("black") );
			p.drawLine( i, 0, i, h );
		}	
	}
	
	MemChunk *chunk;
};



class MemDialog : public QDialog
{
	Q_OBJECT
public:
	MemDialog( QWidget *parent ) : QDialog( parent ) {
		box = new QBoxLayout( QBoxLayout::TopToBottom );
		check();
		setLayout( box );
		
		connect( &timer, SIGNAL(timeout()), this, SLOT(refresh()) );
		timer.start( 300 );
	}
	
private slots:
	void refresh() {
		check();
		for ( int i = 0; i < memFrames.count(); ++i )
			memFrames[i]->update();
	}
	
private:
	void check() {
		while( !memFrames.isEmpty() )
			delete memFrames.takeFirst();

		BufferPool *p = BufferPool::globalInstance();
		for ( int i = 0; i < 3; ++i ) {
			for ( int j = 0; j < p->chunkList[i]->count(); ++j ) {
				MemoryFrame *m = new MemoryFrame( this, p->chunkList[i]->at( j ) );
				box->addWidget( m );
				memFrames.append( m );
			}
		}
	}
	
	QBoxLayout* box;
	QTimer timer;
	QList<MemoryFrame*> memFrames;
};



class SeekSlider : public QSlider
{
	Q_OBJECT
public:
	explicit SeekSlider(QWidget *parent) : QSlider(parent) { }
	~SeekSlider() { }

private:
	void mousePressEvent(QMouseEvent *event) {
		int buttons = style()->styleHint(QStyle::SH_Slider_AbsoluteSetButtons);
		Qt::MouseButton button = static_cast<Qt::MouseButton>(buttons & (~(buttons - 1)));
		QMouseEvent modifiedEvent(event->type(), event->pos(), event->globalPos(), button,
			event->buttons() ^ event->button() ^ button, event->modifiers());
		QSlider::mousePressEvent(&modifiedEvent);
	}
};



class Thumbnailer;



class TopWindow : public QMainWindow, protected Ui::MainWindow
{
	Q_OBJECT
public:
	TopWindow();
	
	Source* getDroppedCut( int index, QString mime, QString filename, double &start, double &len );
	Sampler* getSampler() { return sampler; };

private slots:
	void showMemoryInfo();
	
	void renderDialog();
	void renderStart( double startPts );
	void renderFinished( double pts );
	
	void openSources();
	void thumbResultReady( ThumbResult result );
	
	void trackRequest( bool rm, int index );
	void clipAddedToTimeline( Profile );
	void projectSettings( int warn = 0 );
	void menuProjectSettings();

	void newProject();
	void saveProject();
	void loadProject();
	
	void ensureVisible( const QGraphicsItem *it );
	void centerOn( const QGraphicsItem *it );
	void showProjectClipsPage();
	void showFxPage();
	void showFxSettingsPage();

	void setThumbContext( QGLWidget* );
	void sourceActivated();
	void currentFramePts( double d );
	void modeSwitched();

	void setInPoint();
	void setOutPoint();

	void composerPaused( bool b );
	void playPause( bool playing );
	void videoPlayPause();
	void playForward();
	void playBackward();
	void playFaster();
	void playSlower();
	void seekPrevious();
	void seekNext();
	void seekBackward();
	void seekForward();
	void seek( int v );
	void timelineSeek( double pts );
	
	void editAnimation( FilterWidget *f, ParameterWidget *pw, Parameter *p );
	void quitEditor();
	
private:
	void unsupportedDuplicateMessage();
	
	ProjectSourcesPage *sourcePage;
	FxPage *fxPage;
	
	TimelineGraphicsView *timelineView;
	Timeline *timeline;
	AnimEditor *animEditor;
	
	VideoWidget *vw;
	Sampler *sampler;

	SeekSlider *seekSlider;	
	
	QString openSourcesCurrentDir;
	QStringList unsupportedOpenSources;
	QStringList duplicateOpenSources;
	int openSourcesCounter;
	
	ProjectFile *projectLoader;
	Thumbnailer *thumbnailer;
	
	Profile tempProfile;
	
signals:
	void startOSDTimer( bool );
	void timelineReadyForEncode();
	void setCursorPos( double );
};
#endif // TOPWINDOW_H