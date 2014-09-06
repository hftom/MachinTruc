#include <QLabel>

#include "engine/filtercollection.h"
#include "gui/fxpage.h"



FxPage::FxPage()
	: currentEffectsWidget( NULL ),
	effectsWidgetLayout( NULL )
{
	setupUi( this );
	
	FilterCollection *fc = FilterCollection::getGlobalInstance();
	int i;
	
	for ( i = 0; i < fc->videoFilters.count(); ++i ) {
		QListWidgetItem *it = new QListWidgetItem( fc->videoFilters[ i ].name );
		it->setData( 100, fc->videoFilters[ i ].identifier );
		listWidget->addItem( it );
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
	
	if ( clip ) {
		int i;
		currentEffectsWidget = new QWidget();
		currentEffectsWidget->setMinimumWidth( 150 );
		effectsWidgetLayout = new QGridLayout( currentEffectsWidget );
		effectsWidgetLayout->setContentsMargins( 0, 0, 0, 0 );
		for ( i = 0; i < clip->videoFilters.count(); ++i ) {
			FilterWidget *fw = new FilterWidget( currentEffectsWidget, clip, clip->videoFilters.at( i ) );
			connect( fw, SIGNAL(filterDeleted(Clip*,Filter*)), this, SLOT(deletedFilter(Clip*,Filter*)) );
			connect( fw, SIGNAL(filterMoveUp(Clip*,Filter*)), this, SLOT(filterMoveUp(Clip*,Filter*)) );
			connect( fw, SIGNAL(filterMoveDown(Clip*,Filter*)), this, SLOT(filterMoveDown(Clip*,Filter*)) );
			connect( fw, SIGNAL(updateFrame()), this, SIGNAL(updateFrame()) );
			connect( fw, SIGNAL(editAnimation(FilterWidget*,ParameterWidget*,Parameter*)), this, SIGNAL(editAnimation(FilterWidget*,ParameterWidget*,Parameter*)) );
			effectsWidgetLayout->addWidget( fw, i, 1 );
			filterWidgets.append( fw );
		}
		effectsWidgetLayout->setRowStretch( i, 1 );
		effectsWidget->setWidget( currentEffectsWidget );
	}
}



void FxPage::deletedFilter( Clip *c, Filter *f )
{
	int i;
	for ( i = 0; i < filterWidgets.count(); ++i ) {
		if ( filterWidgets[i]->getFilter() == f ) {
			emit filterDeleted( c, f );
			delete filterWidgets.takeAt( i );
			break;
		}
	}
}



void FxPage::filterMoveUp( Clip *c, Filter *f )
{
	for ( int i = 0; i < filterWidgets.count(); ++i ) {
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
			break;
		}
	}
}



void FxPage::filterMoveDown( Clip *c, Filter *f )
{
	for ( int i = 0; i < filterWidgets.count(); ++i ) {
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
			break;
		}
	}
}
