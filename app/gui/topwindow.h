#ifndef TOPWINDOW_H
#define TOPWINDOW_H

#include <QMainWindow>
#include <QSlider>
#include <QProgressDialog>
#include <QTimer>

#include "ui_mainwindow.h"

#include "gui/filter/shaderedit.h"
#include "gui/projectclipspage.h"
#include "gui/fxpage.h"
#include "gui/fxsettingspage.h"
#include "gui/appconfig.h"

#include "engine/thumbnailer.h"
#include "engine/composer.h"
#include "videoout/videowidget.h"
#include "timeline/timeline.h"
#include "animation/animeditor.h"
#include "projectfile.h"
#include "clipboard.h"



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
	explicit SeekSlider(QWidget *parent) : QSlider(parent), buttonDown( false ) { }
	~SeekSlider() { }
	bool isButtonDown() { return buttonDown; }

private:
	void mousePressEvent(QMouseEvent *event) {
		buttonDown = true;
		int buttons = style()->styleHint(QStyle::SH_Slider_AbsoluteSetButtons);
		Qt::MouseButton button = static_cast<Qt::MouseButton>(buttons & (~(buttons - 1)));
		QMouseEvent modifiedEvent(event->type(), event->pos(), event->globalPos(), button,
			event->buttons() ^ event->button() ^ button, event->modifiers());
		QSlider::mousePressEvent(&modifiedEvent);
	}
	
	void mouseReleaseEvent(QMouseEvent *event) {
		buttonDown = false;
		QSlider::mouseReleaseEvent( event );
	}
	
	bool buttonDown;
};



class Thumbnailer;



class TopWindow : public QMainWindow, protected Ui::MainWindow
{
	Q_OBJECT
public:
	TopWindow();
	
	Source* getDroppedCut( int index, QString mime, QString filename, double &start, double &len );
	Sampler* getSampler() { return sampler; };
	void timelineTrackAddRemove( int index, bool remove );
	QList<Source*> getAllSources();
	QList<Source*> getBuiltinSources();
	QList<Source*> getSelectedSources();

public slots:
	void clipThumbRequest( ThumbRequest request );
	
protected:
	void closeEvent( QCloseEvent *event );
	void keyPressEvent( QKeyEvent *event );
	void keyReleaseEvent( QKeyEvent *event );

private slots:
	void showMemoryInfo();
	
	bool saveAndContinue();
	void doBackup();
	void loadBackup();
	
	void renderDialog();
	void renderStart( double startPts, QSize out );
	void renderFinished( double pts );
	
	void openSources();
	void thumbResultReady( ThumbRequest result );
	void requestBuiltinThumb(QString name, int type);
	
	void trackRequest( bool rm, int index );
	void clipAddedToTimeline( Profile );
	void projectSettings( int warn = 0 );
	void menuProjectSettings();

	void newProject();
	void saveProject();
	void openProject();
	
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
	void hideAnimEditor(int);
	
	void selectAll();
	void moveMulti();

	void editCopy();
	void editCut();
	void editPaste();

	void zoomIn();
	void zoomOut();
	
	void filterCopy(QSharedPointer<Filter>, bool audio);
	
private:
	bool ignoreBackgroundJobsRunning();
	bool loadProject(QString filename, QString &backupFilename);
	void removeBackup();
	void unsupportedDuplicateMessage();
	
	ProjectSourcesPage *sourcePage;
	FxPage *fxPage;
	FxSettingsPage *fxSettingsPage;
	
	TimelineGraphicsView *timelineView;
	Timeline *timeline;
	AnimEditor *animEditor;
	
	VideoWidget *vw;
	Sampler *sampler;

	SeekSlider *seekSlider;	
	
	QString openSourcesCurrentDir, openProjectCurrentDir, currentProjectFile;
	QStringList unsupportedOpenSources;
	QStringList duplicateOpenSources;
	int openSourcesCounter;
	
	ProjectFile *projectLoader;
	Thumbnailer *thumbnailer;
	
	Profile tempProfile;
	AppConfig appConfig;
	QTimer backupTimer;
	
	ClipBoard *clipboard;
	
	QUndoStack undoStack;
	
signals:
	void startOSDTimer( bool );
	void timelineReadyForEncode();
	void setCursorPos( double pts, bool isPlaying );
};
#endif // TOPWINDOW_H
