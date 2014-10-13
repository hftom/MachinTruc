#include <QLabel>

#include "engine/filtercollection.h"
#include "gui/fxpage.h"



FxPage::FxPage()
	: currentEffectsWidget( NULL ),
	effectsWidgetLayout( NULL ),
	currentEffectsWidgetAudio( NULL ),
	effectsWidgetLayoutAudio( NULL )
{
	setupUi( this );
	
	FilterCollection *fc = FilterCollection::getGlobalInstance();
	int i;
	
	for ( i = 0; i < fc->videoFilters.count(); ++i ) {
		QListWidgetItem *it = new QListWidgetItem( fc->videoFilters[ i ].name );
		it->setData( 100, fc->videoFilters[ i ].identifier );
		videoListWidget->addItem( it );
	}
	
	for ( i = 0; i < fc->audioFilters.count(); ++i ) {
		QListWidgetItem *it = new QListWidgetItem( fc->audioFilters[ i ].name );
		it->setData( 100, fc->audioFilters[ i ].identifier );
		audioListWidget->addItem( it );
	}
}



void FxPage::clipSelected( Clip *clip )
{
	if ( currentEffectsWidget ) {
		delete currentEffectsWidget;
		currentEffectsWidget = NULL;
		effectsWidgetLayout = NULL;
		filterWidgets.clear();
	}
	
	if ( currentEffectsWidgetAudio ) {
		delete currentEffectsWidgetAudio;
		currentEffectsWidgetAudio = NULL;
		effectsWidgetLayoutAudio = NULL;
		filterWidgetsAudio.clear();
	}
	
	if ( clip ) {
		int i;
		currentEffectsWidget = new QWidget();
		currentEffectsWidget->setMinimumWidth( 150 );
		effectsWidgetLayout = new QGridLayout( currentEffectsWidget );
		effectsWidgetLayout->setContentsMargins( 0, 0, 0, 0 );
		for ( i = 0; i < clip->videoFilters.count(); ++i ) {
			FilterWidget *fw = new FilterWidget( currentEffectsWidget, clip, clip->videoFilters.at( i ) );
			connect( fw, SIGNAL(filterDeleted(Clip*,QSharedPointer<Filter>)), this, SLOT(deletedFilter(Clip*,QSharedPointer<Filter>)) );
			connect( fw, SIGNAL(filterMoveUp(Clip*,QSharedPointer<Filter>)), this, SLOT(filterMoveUp(Clip*,QSharedPointer<Filter>)) );
			connect( fw, SIGNAL(filterMoveDown(Clip*,QSharedPointer<Filter>)), this, SLOT(filterMoveDown(Clip*,QSharedPointer<Filter>)) );
			connect( fw, SIGNAL(updateFrame()), this, SIGNAL(updateFrame()) );
			connect( fw, SIGNAL(editAnimation(FilterWidget*,ParameterWidget*,Parameter*)), this, SIGNAL(editAnimation(FilterWidget*,ParameterWidget*,Parameter*)) );
			effectsWidgetLayout->addWidget( fw, i, 1 );
			filterWidgets.append( fw );
		}
		effectsWidgetLayout->setRowStretch( i, 1 );
		videoEffectsWidget->setWidget( currentEffectsWidget );
		
		currentEffectsWidgetAudio = new QWidget();
		currentEffectsWidgetAudio->setMinimumWidth( 150 );
		effectsWidgetLayoutAudio = new QGridLayout( currentEffectsWidgetAudio );
		effectsWidgetLayoutAudio->setContentsMargins( 0, 0, 0, 0 );
		for ( i = 0; i < clip->audioFilters.count(); ++i ) {
			FilterWidget *fw = new FilterWidget( currentEffectsWidgetAudio, clip, clip->audioFilters.at( i ) );
			connect( fw, SIGNAL(filterDeleted(Clip*,QSharedPointer<Filter>)), this, SLOT(deletedFilter(Clip*,QSharedPointer<Filter>)) );
			connect( fw, SIGNAL(filterMoveUp(Clip*,QSharedPointer<Filter>)), this, SLOT(filterMoveUp(Clip*,QSharedPointer<Filter>)) );
			connect( fw, SIGNAL(filterMoveDown(Clip*,QSharedPointer<Filter>)), this, SLOT(filterMoveDown(Clip*,QSharedPointer<Filter>)) );
			connect( fw, SIGNAL(updateFrame()), this, SIGNAL(updateFrame()) );
			connect( fw, SIGNAL(editAnimation(FilterWidget*,ParameterWidget*,Parameter*)), this, SIGNAL(editAnimation(FilterWidget*,ParameterWidget*,Parameter*)) );
			effectsWidgetLayoutAudio->addWidget( fw, i, 1 );
			filterWidgetsAudio.append( fw );
		}
		effectsWidgetLayoutAudio->setRowStretch( i, 1 );
		audioEffectsWidget->setWidget( currentEffectsWidgetAudio );
	}
}



void FxPage::deletedFilter( Clip *c, QSharedPointer<Filter> f )
{
	int i;
	for ( i = 0; i < filterWidgets.count(); ++i ) {
		if ( filterWidgets[i]->getFilter() == f ) {
			emit filterDeleted( c, f );
			delete filterWidgets.takeAt( i );
			return;
		}
	}
	
	for ( i = 0; i < filterWidgetsAudio.count(); ++i ) {
		if ( filterWidgetsAudio[i]->getFilter() == f ) {
			emit filterDeleted( c, f );
			delete filterWidgetsAudio.takeAt( i );
			return;
		}
	}
}



void FxPage::filterMoveUp( Clip *c, QSharedPointer<Filter> f )
{
	int i;
	for ( i = 0; i < filterWidgets.count(); ++i ) {
		if ( filterWidgets[i]->getFilter() == f ) {
			if ( i == 0 )
				return;
			c->videoFilters.swap( i, i - 1 );
			filterWidgets.swap( i, i - 1 );
			effectsWidgetLayout->removeWidget( filterWidgets[i] );
			effectsWidgetLayout->removeWidget( filterWidgets[i - 1] );
			effectsWidgetLayout->addWidget( filterWidgets[i], i, 1 );
			effectsWidgetLayout->addWidget( filterWidgets[i - 1], i - 1, 1 );
			emit updateFrame();
			return;
		}
	}
	
	for ( i = 0; i < filterWidgetsAudio.count(); ++i ) {
		if ( filterWidgetsAudio[i]->getFilter() == f ) {
			if ( i == 0 )
				return;
			c->audioFilters.swap( i, i - 1 );
			filterWidgetsAudio.swap( i, i - 1 );
			effectsWidgetLayoutAudio->removeWidget( filterWidgetsAudio[i] );
			effectsWidgetLayoutAudio->removeWidget( filterWidgetsAudio[i - 1] );
			effectsWidgetLayoutAudio->addWidget( filterWidgetsAudio[i], i, 1 );
			effectsWidgetLayoutAudio->addWidget( filterWidgetsAudio[i - 1], i - 1, 1 );
			emit updateFrame();
			return;
		}
	}
}



void FxPage::filterMoveDown( Clip *c, QSharedPointer<Filter> f )
{
	int i;
	for ( i = 0; i < filterWidgets.count(); ++i ) {
		if ( filterWidgets[i]->getFilter() == f ) {
			if ( i > filterWidgets.count() - 2 )
				return;
			c->videoFilters.swap( i, i + 1 );
			filterWidgets.swap( i, i + 1 );
			effectsWidgetLayout->removeWidget( filterWidgets[i] );
			effectsWidgetLayout->removeWidget( filterWidgets[i + 1] );
			effectsWidgetLayout->addWidget( filterWidgets[i], i, 1 );
			effectsWidgetLayout->addWidget( filterWidgets[i + 1], i + 1, 1 );
			emit updateFrame();
			return;
		}
	}
	
	for ( i = 0; i < filterWidgetsAudio.count(); ++i ) {
		if ( filterWidgetsAudio[i]->getFilter() == f ) {
			if ( i > filterWidgetsAudio.count() - 2 )
				return;
			c->audioFilters.swap( i, i + 1 );
			filterWidgetsAudio.swap( i, i + 1 );
			effectsWidgetLayoutAudio->removeWidget( filterWidgetsAudio[i] );
			effectsWidgetLayoutAudio->removeWidget( filterWidgetsAudio[i + 1] );
			effectsWidgetLayoutAudio->addWidget( filterWidgetsAudio[i], i, 1 );
			effectsWidgetLayoutAudio->addWidget( filterWidgetsAudio[i + 1], i + 1, 1 );
			emit updateFrame();
			return;
		}
	}
}
