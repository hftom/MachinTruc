#ifndef TOPWINDOW_H
#define TOPWINDOW_H

#include <QMainWindow>
#include <QSlider>

#include "ui_mainwindow.h"

#include "gui/projectclipspage.h"
#include "gui/fxpage.h"
#include "gui/fxsettingspage.h"

#include "engine/composer.h"
#include "videoout/videowidget.h"
#include "timeline/timeline.h"
#include "animation/animeditor.h"



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



class TopWindow : public QMainWindow, protected Ui::MainWindow
{
	Q_OBJECT
public:
	TopWindow();
	
	Source* getDroppedCut( int index, QString mime, QString filename, double &start, double &len );
	Sampler* getSampler() { return sampler; };

private slots:
	void ensureVisible( const QGraphicsItem *it );
	void centerOn( const QGraphicsItem *it );
	void showProjectClipsPage();
	void showFxPage();
	void showFxSettingsPage();

	void sourceActivated( SourceListItem *item );
	void currentFramePts( double d );
	void modeSwitched();

	void setInPoint();
	void setOutPoint();

	void composerPaused( bool b );
	void playPause( bool playing );
	void videoPlayPause();
	void seekPrevious();
	void seekNext();
	void seekBackward();
	void seekForward();
	void seek( int v );
	void timelineSeek( double pts );
	
	void editAnimation( FilterWidget *f, ParameterWidget *pw, Parameter *p );
	void quitEditor();
	
private:
	ProjectSourcesPage *sourcePage;
	FxPage *fxPage;
	SourceListItem *activeSource;
	
	TimelineGraphicsView *timelineView;
	Timeline *timeline;
	AnimEditor *animEditor;
	
	VideoWidget *vw;
	Sampler *sampler;

	SeekSlider *seekSlider;	
	
signals:
	void setCursorPos( double );
};
#endif // TOPWINDOW_H