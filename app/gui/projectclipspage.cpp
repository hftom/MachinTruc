#include <QMenu>
#include <QProgressDialog>
#include <QMessageBox>
#include <QFileDialog>

#include "engine/movitchain.h"
#include "engine/filtercollection.h"

#include <QGLFramebufferObject>

#include "input/input_ff.h"
#include "input/input_image.h"
#include "gui/projectclipspage.h"
#include "gui/profiledialog.h"
#include "gui/filtersdialog.h"



ProjectClipsPage::ProjectClipsPage( Sampler *samp )
{
	sampler = samp;
	
	setupUi( this );
	
	cutListView->setModel( &model );
	cutListView->setViewMode( QListView::IconMode );
	cutListView->setResizeMode( QListView::Adjust );
	cutListView->setSpacing( 4 );
	cutListView->setDragEnabled( true );
	cutListView->setAcceptDrops( false );
	
	sourceListWidget->setIconSize( QSize( ICONSIZEWIDTH + 4, ICONSIZEHEIGHT + 4 ) );
	sourceListWidget->setContextMenuPolicy( Qt::CustomContextMenu );
	connect( sourceListWidget, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(sourceItemMenu(const QPoint&)) );
	connect( sourceListWidget, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)), this, SLOT(sourceItemActivated(QListWidgetItem*,QListWidgetItem*)) );

	connect( openClipToolButton, SIGNAL(clicked()), this, SLOT(openSources()) );
}



Source* ProjectClipsPage::getSource( int index, const QString &filename )
{
	if ( index < 0 || index > sourceListWidget->count() - 1 )
		return NULL;
	SourceListItem *it = (SourceListItem*)sourceListWidget->item( index );
	if ( !it )
		return NULL;
	/*if ( it->getSource()->getFileName() != filename )
		return NULL;*/
	
	return it->getSource();
}



void ProjectClipsPage::sourceItemActivated( QListWidgetItem *item, QListWidgetItem* )
{
	SourceListItem *it = (SourceListItem*)item;
	emit sourceActivated( it );
	model.setSource( it );
	cutListView->setIconSize( it->getThumbSize() );
	cutListView->reset();
}



void ProjectClipsPage::newCut( SourceListItem* item )
{
	item->addCut();
	cutListView->reset();
}



void ProjectClipsPage::sourceItemMenu( const QPoint &pos )
{
	SourceListItem *item = (SourceListItem*)sourceListWidget->itemAt( pos );
	if ( !item )
		return;
	
	QMenu menu;
	menu.addAction( tr("Source properties"), this, SLOT(showSourceProperties()) );
	menu.addAction( tr("Filters..."), this, SLOT(showSourceFilters()) );
	menu.exec( QCursor::pos() );
}



void ProjectClipsPage::showSourceProperties()
{
	SourceListItem *item = (SourceListItem*)sourceListWidget->currentItem();

	if ( !item )
		return;

	ProfileDialog *box = new ProfileDialog( this, item->getFileName(), item->getProfile() );
	box->move( QCursor::pos() );
	box->exec();
}



void ProjectClipsPage::showSourceFilters()
{
	SourceListItem *item = (SourceListItem*)sourceListWidget->currentItem();
	if ( !item )
		return;
	
	FiltersDialog( this, item->getSource(), sampler ).exec();
}



void ProjectClipsPage::openSources()
{
	QStringList	list = QFileDialog::getOpenFileNames( this, tr("Open files"), sourceCurrentDir,
		"Videos(*.dv *.m2t *.mts *.mkv *.mpg *.mpeg *.ts *.avi *.mov *.vob *.wmv *.mjpg *.mp4 *.ogg *.wav *.mp3 *.ac3 *.mp2 *.mpa *.mpc *.png *.jpg)" );

	if ( !list.isEmpty() ) {
		QProgressDialog progress( tr("Loading..."), QString(), 0, list.count(), this );
		progress.setWindowModality( Qt::WindowModal );
		progress.setMinimumDuration( 0 );
		progress.setValue( 1 );
		qApp->processEvents();
		int i, j;
		QStringList unsupported;
		sourceCurrentDir = QFileInfo( list[0] ).absolutePath();
		for ( i = 0; i < list.count(); ++i ) {
			bool skip = false;
			for ( j = 0; j < sourceListWidget->count(); ++j ) {
				SourceListItem *clip = (SourceListItem*)sourceListWidget->item( j );
				if ( clip->getFileName() == list[i] ) {
					skip = true;
					break;
				}
			}
			if ( !skip ) {
				skip = true;
				Profile prof;
				InputBase *input = new InputImage();
				if ( !input->probe( list[ i ], &prof ) ) {
					delete input;
					input = new InputFF();
				}
				if ( input->probe( list[ i ], &prof ) ) {
					QPixmap pix;
					if ( prof.hasVideo() )
						pix = getSourceThumb( input->getVideoFrame() );
					else {
						QImage img( ICONSIZEWIDTH + 4, ICONSIZEHEIGHT + 4, QImage::Format_ARGB32 );
						img.fill( QColor(0,0,0,0) );
						QPainter p(&img);
						p.drawImage( 2, 2, QImage(":/images/icons/sound.png") );
						pix = QPixmap::fromImage( img );
					}			
					Source *src = new Source( input->getType(), list[i], prof );
					SourceListItem *it = new SourceListItem( pix, src );
					sourceListWidget->addItem( it );
					skip = false;
				}
				if ( input )
					delete input;
			}
			if ( skip )
				unsupported.append( QFileInfo( list[i] ).fileName() );
			progress.setValue( i );
			qApp->processEvents();
		}
		progress.setValue( i );

		if ( unsupported.count() ) {
			QMessageBox msgBox;
			msgBox.setText( tr("Some files are unsupported or already part of the project.") );
			msgBox.setDetailedText( unsupported.join( "\n" ) );
			msgBox.exec();
		}
	}
}



QPixmap ProjectClipsPage::getSourceThumb( Frame *f )
{
	if ( !f )
		return QPixmap();
	
	f->glWidth = f->profile.getVideoWidth();
	f->glHeight = f->profile.getVideoHeight();
	
	hidden->makeCurrent();

	MovitChain *movitChain = new MovitChain();
	movitChain->chain = new EffectChain( f->profile.getVideoAspectRatio(), 1.0 );
	MovitInput *in = new MovitInput();
	MovitBranch *branch = new MovitBranch( in );
	movitChain->branches.append( branch );
	movitChain->chain->add_input( in->getMovitInput( f ) );
	branch->input->process( f );
			
	// deinterlace
	if ( f->profile.getVideoInterlaced() ) {
		GLDeinterlace *deint = new GLDeinterlace();
		Effect *e = deint->getMovitEffect();
		branch->filters.append( new MovitFilter( e, deint, true ) );
		movitChain->chain->add_effect( e );
		deint->process( e, f, &f->profile );
	}
			
	int iw, ih;
	if ( f->profile.getVideoAspectRatio() >= ICONSIZEWIDTH / ICONSIZEHEIGHT ) {
		iw = ICONSIZEWIDTH;
		ih = iw / f->profile.getVideoAspectRatio();
	}
	else {
		ih = ICONSIZEHEIGHT;
		iw = ih * f->profile.getVideoAspectRatio();
	}
	// resize
	GLResize *resize = new GLResize();
	resize->width = iw;
	resize->height = ih;
	Effect *e = resize->getMovitEffect();
	resize->process( e, f, &f->profile );
	branch->filters.append( new MovitFilter( e, resize, true ) );
	movitChain->chain->add_effect( e );
	
	movitChain->chain->set_dither_bits( 0 );
	ImageFormat output_format;
	output_format.color_space = COLORSPACE_sRGB;
	output_format.gamma_curve = GAMMA_sRGB;
	movitChain->chain->add_output( output_format, OUTPUT_ALPHA_FORMAT_POSTMULTIPLIED );
	movitChain->chain->finalize();
	
	// render
	GLuint fbo, tex;
	glGenFramebuffers( 1, &fbo );
	glGenTextures( 1, &tex );
	glActiveTexture( GL_TEXTURE0 );
    glBindTexture( GL_TEXTURE_2D, tex );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, iw, ih, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glBindFramebuffer( GL_FRAMEBUFFER, fbo );
    glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0 );
	
	movitChain->chain->render_to_fbo( fbo, iw, ih );
	
	uint8_t data[iw*ih*4];
	glBindFramebuffer( GL_FRAMEBUFFER, fbo );
	glReadPixels(0, 0, iw, ih, GL_BGRA, GL_UNSIGNED_BYTE, data);
	glBindFramebuffer( GL_FRAMEBUFFER, 0 );
	
	QImage img( ICONSIZEWIDTH + 4, ICONSIZEHEIGHT + 4, QImage::Format_ARGB32 );
	img.fill( QColor(0,0,0,0) );
	QPainter p(&img);
	p.drawImage( (ICONSIZEWIDTH - iw) / 2 + 2, (ICONSIZEHEIGHT - ih) / 2 + 2, QImage( data, iw, ih, QImage::Format_ARGB32 ).mirrored() );

	f->release();
	delete movitChain;
	glDeleteFramebuffers( 1, &fbo );
	glDeleteTextures( 1, &tex );
	hidden->doneCurrent();
	
	return QPixmap::fromImage( img );
}



void ProjectClipsPage::setSharedContext( QGLWidget *shared )
{	
	hidden = shared;
}
