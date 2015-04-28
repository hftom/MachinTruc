#ifndef FXPAGE_H
#define FXPAGE_H

#include <QWidget>

#include "ui_fxpage.h"
#include "timeline/clipviewitem.h"
#include "gui/filter/filterwidget.h"
#include "graph.h"



class FxPage : public QWidget, private Ui::StackFx
{
	Q_OBJECT
public:
	FxPage();
	
public slots:
	void clipSelected( ClipViewItem *clip );
	
private slots:
	void tabChanged( int index );
	void videoFilterSelected( Clip *c, int index );
	void audioFilterSelected( Clip *c, int index );
	void showVideoEffect( int index );
	void showAudioEffect( int index );

private:
	QWidget *currentEffectsWidget;
	QGridLayout *effectsWidgetLayout;
	
	QWidget *currentEffectsWidgetAudio;
	QGridLayout *effectsWidgetLayoutAudio;
	
	Graph *videoGraph;
	Graph *audioGraph;
	
signals:
	void filterDeleted( Clip*, QSharedPointer<Filter> );
	void filterAdded( ClipViewItem*, QString, int );
	void editAnimation( FilterWidget*, ParameterWidget*, Parameter* );
	void updateFrame();
	void showEffect( bool isVideo, int index );
	void currentFilterChanged( int );
	
	void ovdValueChanged();
};

#endif // FXPAGE_H