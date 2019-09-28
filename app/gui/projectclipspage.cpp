#include <QMenu>

#include <movit/resample_effect.h>
#include "engine/movitchain.h"
#include "engine/filtercollection.h"

#include <QGLFramebufferObject>

#include "input/input_ff.h"
#include "input/input_image.h"
#include "engine/thumbnailer.h"
#include "gui/projectclipspage.h"
#include "gui/profiledialog.h"
#include "gui/filtersdialog.h"



ProjectSourcesPage::ProjectSourcesPage( Sampler *samp )
	: sampler( samp ),
	activeSource( NULL )
{	
	setupUi( this );
	
	sourceListWidget->setIconSize( QSize( ICONSIZEWIDTH + 4, ICONSIZEHEIGHT + 4 ) );
	sourceListWidget->setContextMenuPolicy( Qt::CustomContextMenu );
	connect( sourceListWidget, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(sourceItemMenu(const QPoint&)) );
	connect( sourceListWidget, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)), this, SLOT(sourceItemActivated(QListWidgetItem*,QListWidgetItem*)) );

	connect( openClipToolButton, SIGNAL(clicked()), this, SIGNAL(openSourcesBtnClicked()) );
	connect( openBlankToolButton, SIGNAL(clicked()), this, SIGNAL(openBlankBtnClicked()) );
}



bool ProjectSourcesPage::exists( QString name )
{
	for ( int j = 0; j < sourceListWidget->count(); ++j ) {
		SourceListItem *clip = (SourceListItem*)sourceListWidget->item( j );
		if ( clip->getFileName() == name )
			return true;
	}
	return false;
}



QList<Source*> ProjectSourcesPage::getAllSources()
{
	QList<Source*> list;
	for ( int i = 0; i < sourceListWidget->count(); ++i ) {
		SourceListItem *it = (SourceListItem*)sourceListWidget->item( i );
		list.append( it->getSource() );
	}
	
	return list;
}



void ProjectSourcesPage::clearAllSources()
{
	for ( int i = 0; i < sourceListWidget->count(); ++i ) {
		SourceListItem *it = (SourceListItem*)sourceListWidget->item( i );
		it->getSource()->release();
	}
	activeSource = NULL;
	sourceListWidget->clear();
}



Source* ProjectSourcesPage::getSource( int index, const QString &filename )
{
	Q_UNUSED( filename );
	if ( index < 0 || index > sourceListWidget->count() - 1 )
		return NULL;
	SourceListItem *it = (SourceListItem*)sourceListWidget->item( index );
	if ( !it )
		return NULL;
	/*if ( it->getSource()->getFileName() != filename )
		return NULL;*/
	
	return it->getSource();
}



QList<Source*> ProjectSourcesPage::getSelectedSources()
{
	QList<QListWidgetItem*> list = sourceListWidget->selectedItems();
	QList<Source*> res;
	for (int i = 0; i < list.count(); ++i) {
		SourceListItem *it = (SourceListItem*)list.at(i);
		res.append(it->getSource());
	}
	return res;
}


void ProjectSourcesPage::sourceItemActivated( QListWidgetItem *item, QListWidgetItem *prev )
{
	Q_UNUSED( prev );
	if ( !item )
		return;

	SourceListItem *it = (SourceListItem*)item;
	activeSource = it;
	emit sourceActivated();
}



void ProjectSourcesPage::sourceItemMenu( const QPoint &pos )
{
	SourceListItem *item = (SourceListItem*)sourceListWidget->itemAt( pos );
	if ( !item )
		return;
	
	QMenu menu;
	if (sourceListWidget->selectedItems().count() > 1) {
		menu.addAction( tr("Add selection to track..."), this, SIGNAL(addSelectionToTimeline()) );
	}
	menu.addAction( tr("Source properties"), this, SLOT(showSourceProperties()) );
	menu.addAction( tr("Filters..."), this, SLOT(showSourceFilters()) );
	menu.exec( QCursor::pos() );
}



void ProjectSourcesPage::showSourceProperties()
{
	SourceListItem *item = (SourceListItem*)sourceListWidget->currentItem();

	if ( !item )
		return;

	ProfileDialog *box = new ProfileDialog( this, item->getFileName(), item->getProfile() );
	box->move( QCursor::pos() );
	box->exec();
	delete box;
}



void ProjectSourcesPage::showSourceFilters()
{
	SourceListItem *item = (SourceListItem*)sourceListWidget->currentItem();
	if ( !item )
		return;
	
	FiltersDialog( this, item->getSource(), sampler ).exec();
}



void ProjectSourcesPage::addSource( QPixmap pix, Source *src )
{
	SourceListItem *it = new SourceListItem( pix, src );
	sourceListWidget->addItem( it );
}
