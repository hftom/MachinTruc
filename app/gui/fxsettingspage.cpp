#include "engine/filtercollection.h"

#include "gui/fxsettingspage.h"



FxSettingsPage::FxSettingsPage()
{
	setupUi( this );
	
	/*FilterCollection *fc = FilterCollection::getGlobalInstance();
	
	for ( int i = 0; i < fc->videoTransitions.count(); ++i ) {
		videoFiltersCombo->addItem( fc->videoTransitions[ i ].name );
	}
	
	for ( int i = 0; i < fc->audioTransitions.count(); ++i ) {
		audioFiltersCombo->addItem( fc->audioTransitions[ i ].name );
	}*/
}
