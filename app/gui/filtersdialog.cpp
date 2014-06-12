#include "engine/filtercollection.h"
#include "filtersdialog.h"



FiltersDialog::FiltersDialog( QWidget *parent, Source *src, Sampler *samp ) : QDialog( parent ) 
{
	setupUi( this );
	currentVideoWidget = NULL;
	currentAudioWidget = NULL;
		
	source = src;
	sampler = samp;
	
	if ( source->getProfile().hasVideo() ) {
		videoList->addItems( source->videoFilters.videoFiltersNames() );	
		connect( videoList, SIGNAL(currentRowChanged(int)), this, SLOT(videoFilterActivated(int)) );
		connect( videoAddBtn, SIGNAL(clicked()), this, SLOT(showVideoFiltersList()) );
		connect( videoRemoveBtn, SIGNAL(clicked()), this, SLOT(removeCurrentVideoFilter()) );
	}
	else {
		tabWidget->setCurrentWidget( audioTab );
		videoTab->setEnabled( false );
	}
	
	if ( source->getProfile().hasAudio() ) {
		audioList->addItems( source->audioFilters.audioFiltersNames() );
		connect( audioList, SIGNAL(currentRowChanged(int)), this, SLOT(audioFilterActivated(int)) );
		connect( audioAddBtn, SIGNAL(clicked()), this, SLOT(showAudioFiltersList()) );
		connect( audioRemoveBtn, SIGNAL(clicked()), this, SLOT(removeCurrentAudioFilter()) );
	}
	else
		audioTab->setEnabled( false );
	
	move( QPoint(0,0) );
}



FiltersDialog::~FiltersDialog()
{
	if ( currentVideoWidget )
		delete currentVideoWidget;
	if ( currentAudioWidget )
		delete currentAudioWidget;
}



void FiltersDialog::videoFilterActivated( int row )
{
	if ( currentVideoWidget ) {
		delete currentVideoWidget;
		currentVideoWidget = NULL;
	}
	
	if ( row < 0 || row >= source->videoFilters.count() ) // list was cleared
		return;
	
	currentVideoWidget = source->videoFilters.at( row )->getWidget();
	videoWidgetLayout->addWidget( currentVideoWidget );
}



void FiltersDialog::showVideoFiltersList()
{
	FiltersListDlg *dlg = new FiltersListDlg( MODEVIDEO, this );
	connect( dlg, SIGNAL(addVideoFilter(int)), this, SLOT(addVideoFilter(int)) );
	dlg->exec();
}



void FiltersDialog::addVideoFilter( int i )
{
	FilterCollection *fc = FilterCollection::getGlobal();
	
	GLFilter *f = (GLFilter*)fc->videoFilters.at( i ).makeFilter( fc->videoFilters.at( i ).identifier, fc->videoFilters.at( i ).name );
	connect( f, SIGNAL(updateFrame()), sampler, SLOT(updateFrame()) );
	source->videoFilters.append( f );
	videoList->clear();
	videoList->addItems( source->videoFilters.videoFiltersNames() );
	videoList->setCurrentRow( videoList->count() - 1 );
	sampler->updateFrame();
}



void FiltersDialog::removeCurrentVideoFilter()
{
	QListWidgetItem *it = videoList->currentItem();
	if ( !it )
		return;
	int row = videoList->row( it );
	if ( row < 0 || row >= source->videoFilters.count() )
		return;
	GLFilter *f = source->videoFilters.at( row );
	source->videoFilters.remove( f );
	
	videoList->clear();
	videoList->addItems( source->videoFilters.videoFiltersNames() );
	if ( videoList->count() )
		videoList->setCurrentRow( 0 );
	else if ( currentVideoWidget ) {
		delete currentVideoWidget;
		currentVideoWidget = NULL;
	}
	
	sampler->updateFrame();
}



void FiltersDialog::audioFilterActivated( int row )
{
	if ( currentAudioWidget ) {
		delete currentAudioWidget;
		currentAudioWidget = NULL;
	}
	
	if ( row < 0 || row >= source->audioFilters.count() ) // list was cleared
		return;
	
	currentAudioWidget = source->audioFilters.at( row )->getWidget();
	audioWidgetLayout->addWidget( currentAudioWidget );
}



void FiltersDialog::showAudioFiltersList()
{
	FiltersListDlg *dlg = new FiltersListDlg( MODEAUDIO, this );
	connect( dlg, SIGNAL(addAudioFilter(int)), this, SLOT(addAudioFilter(int)) );
	dlg->exec();
}



void FiltersDialog::addAudioFilter( int i )
{
	FilterCollection *fc = FilterCollection::getGlobal();
	
	AudioFilter *f = (AudioFilter*)fc->audioFilters.at( i ).makeFilter( fc->audioFilters.at( i ).identifier, fc->audioFilters.at( i ).name );
	source->audioFilters.append( f );
	audioList->clear();
	audioList->addItems( source->audioFilters.audioFiltersNames() );
	audioList->setCurrentRow( audioList->count() - 1 );
}



void FiltersDialog::removeCurrentAudioFilter()
{
	QListWidgetItem *it = audioList->currentItem();
	if ( !it )
		return;
	int row = audioList->row( it );
	if ( row < 0 || row >= source->audioFilters.count() )
		return;
	AudioFilter *f = source->audioFilters.at( row );
	source->audioFilters.remove( f );
	
	audioList->clear();
	audioList->addItems( source->audioFilters.audioFiltersNames() );
	if ( audioList->count() )
		audioList->setCurrentRow( 0 );
	else if ( currentAudioWidget ) {
		delete currentAudioWidget;
		currentAudioWidget = NULL;
	}
}






FiltersListDlg::FiltersListDlg( int m, QWidget *parent ) : QDialog( parent )
{
	mode = m;
	
	FilterCollection *fc = FilterCollection::getGlobal();
	int i;
	
	if ( mode == MODEVIDEO )
		setWindowTitle( tr("Video Filters") );
	else
		setWindowTitle( tr("Audio Filters") );
	QBoxLayout *layout = new QBoxLayout( QBoxLayout::TopToBottom, this );
	QListWidget *list = new QListWidget( this );
	layout->addWidget( list );
	if ( mode == MODEVIDEO ) {
		for ( i = 0; i < fc->videoFilters.count(); ++i )
			list->addItem( new QListWidgetItem( fc->videoFilters.at( i ).name ) );
	}
	else {
		for ( i = 0; i < fc->audioFilters.count(); ++i )
			list->addItem( new QListWidgetItem( fc->audioFilters.at( i ).name ) );
	}
	
	connect( list, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(filterSelected(QListWidgetItem*)) );
	
	move( QCursor::pos() );
}



void FiltersListDlg::filterSelected( QListWidgetItem *item )
{
	FilterCollection *fc = FilterCollection::getGlobal();
	int i;

	if ( mode == MODEVIDEO ) {
		for ( i = 0; i < fc->videoFilters.count(); ++i ) {
			if ( item->text() == fc->videoFilters.at( i ).name ) {
				emit addVideoFilter( i );
				accept();
			}
		}
	}
	else {
		for ( i = 0; i < fc->audioFilters.count(); ++i ) {
			if ( item->text() == fc->audioFilters.at( i ).name ) {
				emit addAudioFilter( i );
				accept();
			}
		}
	}
}
