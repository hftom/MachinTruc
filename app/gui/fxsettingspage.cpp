#include "engine/filtercollection.h"

#include "gui/filter/filterwidget.h"
#include "gui/fxsettingspage.h"



FxSettingsPage::FxSettingsPage()
	: currentEffectWidget( NULL ),
	effectWidgetLayout( NULL ),
	currentEffectWidgetAudio( NULL ),
	effectWidgetLayoutAudio( NULL ),
	currentClip( NULL )
{
	setupUi( this );
	
	FilterCollection *fc = FilterCollection::getGlobalInstance();
	
	for ( int i = 0; i < fc->videoTransitions.count(); ++i ) {
		videoFiltersCombo->addItem( fc->videoTransitions[ i ].name );
	}
	
	for ( int i = 0; i < fc->audioTransitions.count(); ++i ) {
		audioFiltersCombo->addItem( fc->audioTransitions[ i ].name );
	}
	
	connect( videoFiltersCombo, SIGNAL(activated(const QString&)), this, SLOT(videoFilterActivated(const QString&)) );
	connect( audioFiltersCombo, SIGNAL(activated(const QString&)), this, SLOT(audioFilterActivated(const QString&)) );
}



void FxSettingsPage::clipSelected( ClipViewItem *clip )
{
	currentClip = clip;
	if ( currentEffectWidget ) {
		delete currentEffectWidget;
		currentEffectWidget = NULL;
		effectWidgetLayout = NULL;
	}
	if ( currentEffectWidgetAudio ) {
		delete currentEffectWidgetAudio;
		currentEffectWidgetAudio = NULL;
		effectWidgetLayoutAudio = NULL;
	}
	
	if ( clip && clip->getClip()->getTransition() ) {
		videoFiltersCombo->setHidden( false );
		audioFiltersCombo->setHidden( false );
		setComboItems( clip->getClip()->getTransition() );
		
		currentEffectWidget = new QWidget();
		currentEffectWidget->setMinimumWidth( 150 );
		effectWidgetLayout = new QGridLayout( currentEffectWidget );
		effectWidgetLayout->setContentsMargins( 0, 0, 0, 0 );
		FilterWidget *fw = new FilterWidget( currentEffectWidget, clip->getClip(), clip->getClip()->getTransition()->getVideoFilter() );
		connect( fw, SIGNAL(updateFrame()), this, SIGNAL(updateFrame()) );
		connect( fw, SIGNAL(paramUndoCommand(QSharedPointer<Filter>,Parameter*,QVariant,QVariant)), this, SIGNAL(paramUndoCommand(QSharedPointer<Filter>,Parameter*,QVariant,QVariant)) );
		effectWidgetLayout->addWidget( fw, 0, 1 );
		effectWidgetLayout->setRowStretch( 1, 1 );
		videoWidget->setWidget( currentEffectWidget );
		
		currentEffectWidgetAudio = new QWidget();
		currentEffectWidgetAudio->setMinimumWidth( 150 );
		effectWidgetLayoutAudio = new QGridLayout( currentEffectWidgetAudio );
		effectWidgetLayoutAudio->setContentsMargins( 0, 0, 0, 0 );
		fw = new FilterWidget( currentEffectWidgetAudio, clip->getClip(), clip->getClip()->getTransition()->getAudioFilter() );
		connect( fw, SIGNAL(updateFrame()), this, SIGNAL(updateFrame()) );
		connect( fw, SIGNAL(paramUndoCommand(QSharedPointer<Filter>,Parameter*,QVariant,QVariant)), this, SIGNAL(paramUndoCommand(QSharedPointer<Filter>,Parameter*,QVariant,QVariant)) );
		effectWidgetLayoutAudio->addWidget( fw, 0, 1 );
		effectWidgetLayoutAudio->setRowStretch( 1, 1 );
		audioWidget->setWidget( currentEffectWidgetAudio );
	}
	else {
		videoFiltersCombo->setHidden( true );
		audioFiltersCombo->setHidden( true );
	}
}



void FxSettingsPage::setComboItems( Transition *t )
{
	for ( int i = 0; i < videoFiltersCombo->count(); ++i ) {
		if ( videoFiltersCombo->itemText( i ) == t->getVideoFilter()->getFilterName() ) {
			videoFiltersCombo->setCurrentIndex( i );
			break;
		}
	}
	for ( int i = 0; i < audioFiltersCombo->count(); ++i ) {
		if ( audioFiltersCombo->itemText( i ) == t->getAudioFilter()->getFilterName() ) {
			audioFiltersCombo->setCurrentIndex( i );
			break;
		}
	}
}



void FxSettingsPage::videoFilterActivated( const QString& text )
{
	if ( !currentClip )
		return;
	Transition *t = currentClip->getClip()->getTransition();
	if ( !t )
		return;

	if ( t->getVideoFilter()->getFilterName() == text )
		return;
	
	emit transitionChanged(currentClip->getClip(), text, true);
}



void FxSettingsPage::audioFilterActivated( const QString& text )
{
	if ( !currentClip )
		return;
	Transition *t = currentClip->getClip()->getTransition();
	if ( !t )
		return;

	if ( t->getAudioFilter()->getFilterName() == text )
		return;
	
	emit transitionChanged(currentClip->getClip(), text, false);
}
