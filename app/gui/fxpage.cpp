#include <QLabel>

#include "engine/filtercollection.h"
#include "gui/fxpage.h"
#include "effectlistview.h"



FxPage::FxPage()
	: currentEffectsWidget( NULL ),
	effectsWidgetLayout( NULL ),
	currentEffectsWidgetAudio( NULL ),
	effectsWidgetLayoutAudio( NULL )
{
	setupUi( this );
	
	FilterCollection *fc = FilterCollection::getGlobalInstance();
	
	connect( tabWidget, SIGNAL(currentChanged( int)), this, SLOT(tabChanged(int)) );
	
	videoEffectsListView->setModel( new EffectListModel( &fc->videoFilters ) );
	videoGraph = new Graph();
	videoGraphView->setScene( videoGraph );
	connect( videoGraph, SIGNAL(showVerticalScrollBar(bool)), videoGraphView, SLOT(showVerticalScrollBar(bool)) );
	connect( videoGraphView, SIGNAL(sizeChanged(const QSize&)), videoGraph, SLOT(viewSizeChanged(const QSize&)) );
	connect( videoGraph, SIGNAL(filterSelected(Clip*,int)), this, SLOT(videoFilterSelected(Clip*,int)) );
	connect( videoGraph, SIGNAL(filterDeleted(Clip*,QSharedPointer<Filter>)), this, SIGNAL(filterDeleted(Clip*,QSharedPointer<Filter>)) );
	connect( videoGraph, SIGNAL(filterAdded(ClipViewItem*,QString,int)), this, SIGNAL(filterAdded(ClipViewItem*,QString,int)) );
	connect( videoGraph, SIGNAL(updateFrame()), this, SIGNAL(updateFrame()) );
	connect( videoGraph, SIGNAL(showEffect(int)), this, SLOT(showVideoEffect(int)) );
	
	audioEffectsListView->setModel( new EffectListModel( &fc->audioFilters ) );
	audioGraph = new Graph( true );
	audioGraphView->setScene( audioGraph );
	connect( audioGraph, SIGNAL(showVerticalScrollBar(bool)), audioGraphView, SLOT(showVerticalScrollBar(bool)) );
	connect( audioGraphView, SIGNAL(sizeChanged(const QSize&)), audioGraph, SLOT(viewSizeChanged(const QSize&)) );
	connect( audioGraph, SIGNAL(filterSelected(Clip*,int)), this, SLOT(audioFilterSelected(Clip*,int)) );
	connect( audioGraph, SIGNAL(filterDeleted(Clip*,QSharedPointer<Filter>)), this, SIGNAL(filterDeleted(Clip*,QSharedPointer<Filter>)) );
	connect( audioGraph, SIGNAL(filterAdded(ClipViewItem*,QString,int)), this, SIGNAL(filterAdded(ClipViewItem*,QString,int)) );
	connect( audioGraph, SIGNAL(updateFrame()), this, SIGNAL(updateFrame()) );
	connect( audioGraph, SIGNAL(showEffect(int)), this, SLOT(showAudioEffect(int)) );
}



void FxPage::clipSelected( ClipViewItem *clip )
{
	videoGraph->setCurrentClip( clip );
	audioGraph->setCurrentClip( clip );
}


// emit updateFrame() to show/hide OSD
void FxPage::videoFilterSelected( Clip *c, int index )
{
	if ( currentEffectsWidget ) {
		delete currentEffectsWidget;
		currentEffectsWidget = NULL;
		effectsWidgetLayout = NULL;
	}
	
	if ( !c || index < 0 || index >= c->videoFilters.count() ) {
		emit updateFrame();
		return;
	}
	
	currentEffectsWidget = new QWidget();
	effectsWidgetLayout = new QGridLayout( currentEffectsWidget );
	FilterWidget *fw = new FilterWidget( currentEffectsWidget, c, c->videoFilters.at( index ) );
	connect( fw, SIGNAL(updateFrame()), this, SIGNAL(updateFrame()) );
	connect( fw, SIGNAL(editAnimation(FilterWidget*,ParameterWidget*,Parameter*)), this, SIGNAL(editAnimation(FilterWidget*,ParameterWidget*,Parameter*)) );
	connect( this, SIGNAL(ovdValueChanged()), fw, SLOT(ovdValueChanged()) );
	effectsWidgetLayout->addWidget( fw, 0, 1 );
	effectsWidgetLayout->setRowStretch( 1, 1 );
	videoEffectsWidget->setWidget( currentEffectsWidget );
	
	emit updateFrame();
}



void FxPage::audioFilterSelected( Clip *c, int index )
{
	if ( currentEffectsWidgetAudio ) {
		delete currentEffectsWidgetAudio;
		currentEffectsWidgetAudio = NULL;
		effectsWidgetLayoutAudio = NULL;
	}
	
	if ( !c || index < 0 || index >= c->audioFilters.count() )
		return;
	
	currentEffectsWidgetAudio = new QWidget();
	effectsWidgetLayoutAudio = new QGridLayout( currentEffectsWidgetAudio );
	FilterWidget *fw = new FilterWidget( currentEffectsWidgetAudio, c, c->audioFilters.at( index ) );
	connect( fw, SIGNAL(updateFrame()), this, SIGNAL(updateFrame()) );
	connect( fw, SIGNAL(editAnimation(FilterWidget*,ParameterWidget*,Parameter*)), this, SIGNAL(editAnimation(FilterWidget*,ParameterWidget*,Parameter*)) );
	effectsWidgetLayoutAudio->addWidget( fw, 0, 1 );
	effectsWidgetLayoutAudio->setRowStretch( 1, 1 );
	audioEffectsWidget->setWidget( currentEffectsWidgetAudio );
}



void FxPage::tabChanged( int index )
{
	Q_UNUSED( index );
	audioGraph->hiddenEffect();
	videoGraph->hiddenEffect();
	emit showEffect( true, -1 );
}



void FxPage::showVideoEffect( int index )
{
	audioGraph->hiddenEffect();
	emit showEffect( true, index );
}



void FxPage::showAudioEffect( int index )
{
	videoGraph->hiddenEffect();
	emit showEffect( false, index );
}
