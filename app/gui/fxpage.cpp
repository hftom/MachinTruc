#include <QLabel>

#include "engine/filtercollection.h"
#include "gui/fxpage.h"
#include "headereffect.h"



FxPage::FxPage()
{
	setupUi( this );
	
	currentEffectsWidget = NULL;
	
	FilterCollection *fc = FilterCollection::getGlobal();
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
	}
	
	if ( clip ) {
		int i, j = 0;
		currentEffectsWidget = new QWidget();
		QGridLayout *effectsWidgetLayout = new QGridLayout( currentEffectsWidget );
		for ( i = 0; i < clip->videoFilters.count(); ++i ) {
			HeaderEffect *header = new HeaderEffect( clip, clip->videoFilters.at( i ) );
			connect( header, SIGNAL(filterDeleted(Clip*,Filter*)), this, SIGNAL(filterDeleted(Clip*,Filter*)) );
			effectsWidgetLayout->addWidget( header, j++, 1 );
			QWidget *widg = clip->videoFilters.at( i )->getWidget();
			if ( widg )
				effectsWidgetLayout->addWidget( widg, j++, 1 );
		}
		effectsWidgetLayout->setRowStretch( j, 1 );
		effectsWidget->setWidget( currentEffectsWidget );
	}
}
