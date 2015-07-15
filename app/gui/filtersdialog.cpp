#include "engine/filtercollection.h"
#include "filter/filterwidget.h"
#include "filtersdialog.h"



FiltersDialog::FiltersDialog( QWidget *parent, Source *src, Sampler *samp ) : QDialog( parent ),
	source( src ),
	sampler( samp ),
	currentVideoWidget( NULL ),
	currentAudioWidget( NULL )
{
	setupUi( this );
	
	QWidget *videoWidget = new QWidget();
	videoWidgetLayout = new QGridLayout( videoWidget );
	videoParamWidget->setWidget( videoWidget );
	
	QWidget *audioWidget = new QWidget();
	audioWidgetLayout = new QGridLayout( audioWidget );
	audioParamWidget->setWidget( audioWidget );
	
	if ( source->getProfile().hasVideo() ) {
		videoList->addItems( source->videoFilters.filtersNames() );	
		connect( videoList, SIGNAL(currentRowChanged(int)), this, SLOT(videoFilterActivated(int)) );
		connect( videoAddBtn, SIGNAL(clicked()), this, SLOT(showVideoFiltersList()) );
		connect( videoRemoveBtn, SIGNAL(clicked()), this, SLOT(removeCurrentVideoFilter()) );
	}
	else {
		tabWidget->setCurrentWidget( audioTab );
		videoTab->setEnabled( false );
	}
	
	if ( source->getProfile().hasAudio() ) {
		audioList->addItems( source->audioFilters.filtersNames() );
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

	FilterWidget *fw = new FilterWidget( 0, 0, source->videoFilters.at( row ) );
	connect( fw, SIGNAL(updateFrame()), sampler, SLOT(updateFrame()) );
	connect( fw, SIGNAL(filterSourceDeleted()), this, SLOT(removeCurrentVideoFilter()) );
	currentVideoWidget = fw;
	videoWidgetLayout->addWidget( currentVideoWidget, 0, 1 );
	videoWidgetLayout->setRowStretch( 1, 1 );
}



void FiltersDialog::showVideoFiltersList()
{
	FiltersListDlg *dlg = new FiltersListDlg( source->getType(), MODEVIDEO, this );
	connect( dlg, SIGNAL(addVideoFilter(int)), this, SLOT(addVideoFilter(int)) );
	dlg->exec();
}



void FiltersDialog::addVideoFilter( int i )
{
	FilterCollection *fc = FilterCollection::getGlobalInstance();
	
	QSharedPointer<Filter> f = fc->sourceVideoFilters[ i ].create();
	if ( f->getIdentifier() == "GLStabilize"  )
		f.staticCast<GLStabilize>()->setSource( source );
	source->videoFilters.append( f.staticCast<GLFilter>() );
	videoList->clear();
	videoList->addItems( source->videoFilters.filtersNames() );
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
	source->videoFilters.removeAt( row );
	
	videoList->clear();
	videoList->addItems( source->videoFilters.filtersNames() );
	if ( videoList->count() )
		videoList->setCurrentRow( videoList->count() - 1 );
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
	
	FilterWidget *fw = new FilterWidget( 0, 0, source->audioFilters.at( row ) );
	connect( fw, SIGNAL(updateFrame()), sampler, SLOT(updateFrame()) );
	connect( fw, SIGNAL(filterSourceDeleted()), this, SLOT(removeCurrentAudioFilter()) );
	currentAudioWidget = fw;
	audioWidgetLayout->addWidget( currentAudioWidget, 0, 1 );
	audioWidgetLayout->setRowStretch( 1, 1 );	
}



void FiltersDialog::showAudioFiltersList()
{
	FiltersListDlg *dlg = new FiltersListDlg( source->getType(), MODEAUDIO, this );
	connect( dlg, SIGNAL(addAudioFilter(int)), this, SLOT(addAudioFilter(int)) );
	dlg->exec();
}



void FiltersDialog::addAudioFilter( int i )
{
	FilterCollection *fc = FilterCollection::getGlobalInstance();
	
	QSharedPointer<Filter> f = fc->sourceAudioFilters[ i ].create();
	source->audioFilters.append( f.staticCast<AudioFilter>() );
	audioList->clear();
	audioList->addItems( source->audioFilters.filtersNames() );
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
	source->audioFilters.removeAt( row );
	
	audioList->clear();
	audioList->addItems( source->audioFilters.filtersNames() );
	if ( audioList->count() )
		audioList->setCurrentRow( audioList->count() - 1 );
	else if ( currentAudioWidget ) {
		delete currentAudioWidget;
		currentAudioWidget = NULL;
	}
}






FiltersListDlg::FiltersListDlg( int sourceType, int m, QWidget *parent ) : QDialog( parent ),
	mode( m )
{
	FilterCollection *fc = FilterCollection::getGlobalInstance();
	int i;
	
	if ( mode == MODEVIDEO )
		setWindowTitle( tr("Video Filters") );
	else
		setWindowTitle( tr("Audio Filters") );
	QBoxLayout *layout = new QBoxLayout( QBoxLayout::TopToBottom, this );
	QListWidget *list = new QListWidget( this );
	layout->addWidget( list );
	if ( mode == MODEVIDEO ) {
		for ( i = 0; i < fc->sourceVideoFilters.count(); ++i ) {
			if ( fc->sourceVideoFilters.at( i ).identifier == "GLStabilize" && sourceType != InputBase::FFMPEG )
				continue;
			list->addItem( new QListWidgetItem( fc->sourceVideoFilters.at( i ).name ) );
		}
	}
	else {
		for ( i = 0; i < fc->sourceAudioFilters.count(); ++i )
			list->addItem( new QListWidgetItem( fc->sourceAudioFilters.at( i ).name ) );
	}
	
	connect( list, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(filterSelected(QListWidgetItem*)) );
	
	move( QCursor::pos() );
}



void FiltersListDlg::filterSelected( QListWidgetItem *item )
{
	FilterCollection *fc = FilterCollection::getGlobalInstance();
	int i;

	if ( mode == MODEVIDEO ) {
		for ( i = 0; i < fc->sourceVideoFilters.count(); ++i ) {
			if ( item->text() == fc->sourceVideoFilters.at( i ).name ) {
				emit addVideoFilter( i );
				accept();
			}
		}
	}
	else {
		for ( i = 0; i < fc->sourceAudioFilters.count(); ++i ) {
			if ( item->text() == fc->sourceAudioFilters.at( i ).name ) {
				emit addAudioFilter( i );
				accept();
			}
		}
	}
}
