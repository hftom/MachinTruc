#include "gui/topwindow.h"



TopWindow::TopWindow()
{
	setupUi( this );

	activeClip = NULL;

	seekSlider = new SeekSlider( baseVideoWidget );
	seekSlider->setObjectName(QString::fromUtf8("seekSlider"));
	seekSlider->setFocusPolicy(Qt::NoFocus);
	seekSlider->setMaximum(999);
	seekSlider->setPageStep(10);
	seekSlider->setOrientation(Qt::Horizontal);
	seekSlider->setInvertedAppearance(false);
	seekSlider->setInvertedControls(false);
	seekSlider->setTickPosition( QSlider::NoTicks );
	seekSlider->setTickInterval( 0 );
	horizontalLayout_2->insertWidget( 9, seekSlider );
	
	Timeline *timeline = new Timeline( this );
	connect( this, SIGNAL(setCursorPos(double)), timeline, SLOT(setCursorPos(double)) );
	connect( timeline, SIGNAL(seekTo(double)), this, SLOT(timelineSeek(double)) );
	connect( timeline, SIGNAL(ensureVisible(const QGraphicsItem*)), this, SLOT(ensureVisible(const QGraphicsItem*)) );
	connect( timeline, SIGNAL(centerOn(const QGraphicsItem*)), this, SLOT(centerOn(const QGraphicsItem*)) );
	connect( graphicsView, SIGNAL(sizeChanged(const QSize&)), timeline, SLOT(viewSizeChanged(const QSize&)) );

	graphicsView->setAlignment( Qt::AlignLeft | Qt::AlignTop );
	graphicsView->setScene( timeline );
	graphicsView->setAcceptDrops( true );

	sampler = new Sampler();
	
	clipPage = new ProjectClipsPage( sampler );
	connect( clipPage, SIGNAL(sourceActivated(SourceListItem*)), this, SLOT(clipActivated(SourceListItem*)) );
	
	fxPage = new FxPage();
	connect( timeline, SIGNAL(clipSelected(Clip*)), fxPage, SLOT(clipSelected(Clip*)) );
	connect( fxPage, SIGNAL(filterDeleted(Clip*,Filter*)), timeline, SLOT(filterDeleted(Clip*,Filter*)) );
	
	stackedWidget->addWidget( clipPage );
	stackedWidget->addWidget( fxPage );
	stackedWidget->addWidget( new FxSettingsPage() );

	QHBoxLayout *layout = new QHBoxLayout;
	vw = new VideoWidget( glWidget );
	layout->addWidget( vw );
	glWidget->setLayout( layout );

	connect( redoToolButton, SIGNAL(clicked()), vw, SLOT(shot()) );
	connect( vw, SIGNAL(playPause()), this, SLOT(videoPlayPause()) );
	connect( vw, SIGNAL(wheelSeek(int)), sampler, SLOT(wheelSeek(int)) );
	connect( vw, SIGNAL(newSharedContext(QGLWidget*)), sampler, SLOT(setSharedContext(QGLWidget*)) );
	connect( vw, SIGNAL(newFencesContext(QGLWidget*)), sampler, SLOT(setFencesContext(QGLWidget*)) );
	connect( vw, SIGNAL(newThumbContext(QGLWidget*)), clipPage, SLOT(setSharedContext(QGLWidget*)) );
	connect( sampler->getMetronom(), SIGNAL(newFrame(Frame*)), vw, SLOT(showFrame(Frame*)) );
	connect( sampler->getMetronom(), SIGNAL(currentFramePts(double)), this, SLOT(currentFramePts(double)) );
	connect( vw, SIGNAL(frameShown(Frame*)), sampler->getMetronom(), SLOT(setLastFrame(Frame*)) );
	connect( sampler, SIGNAL(newFrame(Frame*)), vw, SLOT(showFrame(Frame*)) );
	connect( sampler, SIGNAL(paused(bool)), this, SLOT(composerPaused(bool)) );
	connect( sampler, SIGNAL(modeSwitched()), this, SLOT(modeSwitched()) );

	connect( clipsToolButton, SIGNAL(clicked()), this, SLOT(showProjectClipsPage()) );
	connect( fxToolButton, SIGNAL(clicked()), this, SLOT(showFxPage()) );
	connect( fxSettingsToolButton, SIGNAL(clicked()), this, SLOT(showFxSettingsPage()) );

	connect( playToolButton, SIGNAL(toggled(bool)), this, SLOT(playPause(bool)) );
	connect( previousToolButton, SIGNAL(clicked()), this, SLOT(seekPrevious()) );
	connect( nextToolButton, SIGNAL(clicked()), this, SLOT(seekNext()) );
	connect( backwardToolButton, SIGNAL(clicked()), this, SLOT(seekBackward()) );
	connect( forwardToolButton, SIGNAL(clicked()), this, SLOT(seekForward()) );

	connect( seekSlider, SIGNAL(valueChanged(int)), this, SLOT(seek(int)) );

	connect( inButton, SIGNAL(clicked()), this, SLOT(setInPoint()) );
	connect( outButton, SIGNAL(clicked()), this, SLOT(setOutPoint()) );

	connect( switchButton, SIGNAL(toggled(bool)), sampler, SLOT(switchMode(bool)) );
	connect( timeline, SIGNAL(updateFrame()), sampler, SLOT(updateFrame()) );
	
	timeline->setScene( sampler->getScene() );

	connect( actionBlackBackground, SIGNAL(toggled(bool)), vw, SLOT(setBlackBackground(bool)) );
	connect( actionDeleteClip, SIGNAL(triggered()), timeline, SLOT(deleteClip()) );
}



void TopWindow::modeSwitched()
{
	if ( activeClip )
		sampler->setClip( activeClip->getSource(), activeClip->getCurrentPts() );
}



void TopWindow::setInPoint()
{
	if ( activeClip ) {
		double pts;
		QImage img = vw->getThumb( THUMBHEIGHT, &pts, false );
		if ( !img.isNull() )
			activeClip->setInPoint( img, pts );
	}
}



void TopWindow::setOutPoint()
{
	if ( activeClip ) {
		double pts;
		QImage img = vw->getThumb( THUMBHEIGHT, &pts, true );
		if ( !img.isNull() ) {
			activeClip->setOutPoint( img, pts );
			clipPage->newCut( activeClip );
		}
	}
}



Source* TopWindow::getDroppedCut( int index, QString mime, QString filename, double &start, double &len )
{
	if ( mime == MIMETYPECUT ) {
		if ( !activeClip )
			return NULL;
		Cut *cut = activeClip->getCut( index, filename );
		if ( !cut )
			return NULL;
		start = cut->getStart();
		len = cut->getLength();
		return cut->getSource();
	}
	if ( mime == MIMETYPESOURCE ) {
		Source *source = clipPage->getSource( index, filename );
		start = source->getProfile().getStreamStartTime();
		len = source->getProfile().getStreamDuration();
		return source;
	}
	return NULL;
}



void TopWindow::currentFramePts( double d )
{
	if ( !switchButton->isChecked() ) {
		if ( !activeClip )
			return;
		activeClip->setCurrentPts( d );
		Profile prof = activeClip->getProfile();
		d -= prof.getStreamStartTime();
		d *= seekSlider->maximum() / prof.getStreamDuration();
	}
	else {
		emit setCursorPos( d ); 
		d *= seekSlider->maximum() / sampler->getSamplerDuration();
	}

	seekSlider->blockSignals( true );
	seekSlider->setValue( d );
	seekSlider->blockSignals( false );
}



void TopWindow::ensureVisible( const QGraphicsItem *it )
{
	graphicsView->ensureVisible( it, 100, 100 );
}



void TopWindow::centerOn( const QGraphicsItem *it )
{
	graphicsView->centerOn( it );
}



void TopWindow::composerPaused( bool b )
{
	// avoid toggled() to be emitted
	playToolButton->blockSignals( true );
	playToolButton->setChecked( !b );
	playToolButton->blockSignals( false );
}



void TopWindow::clipActivated( SourceListItem *item )
{
	if ( item ) {
		activeClip = item;
		if ( switchButton->isChecked() )
			switchButton->toggle();
		else
			sampler->setClip( activeClip->getSource(), activeClip->getCurrentPts() );
		
		//timeline->setCurrentPos( activeClip->getCurrentPts() );
	}
}



void TopWindow::showProjectClipsPage()
{
	stackedWidget->setCurrentIndex( 0 );
}

void TopWindow::showFxPage()
{
	stackedWidget->setCurrentIndex( 1 );
}

void TopWindow::showFxSettingsPage()
{
	stackedWidget->setCurrentIndex( 2 );
}



void TopWindow::playPause( bool playing )
{
	sampler->play( playing );
}

void TopWindow::videoPlayPause()
{
	playToolButton->toggle();
}

void TopWindow::seekPrevious()
{
	sampler->wheelSeek( -1 );
}

void TopWindow::seekNext()
{
	sampler->wheelSeek( 1 );
}

void TopWindow::seekBackward()
{
	sampler->wheelSeek( -10 );
}

void TopWindow::seekForward()
{
	sampler->wheelSeek( 10 );
}

void TopWindow::seek( int v )
{
	double value;
	
	if ( !switchButton->isChecked() ) {
		if ( !activeClip )
			return;
		Profile prof = activeClip->getProfile();
		value = prof.getStreamStartTime();
		value += v * prof.getStreamDuration() / seekSlider->maximum();
	}
	else {
		value = v * sampler->getSamplerDuration() / seekSlider->maximum();
	}

	sampler->slideSeek( value );
}

void TopWindow:: timelineSeek( double pts )
{
	if ( !switchButton->isChecked() )
		switchButton->toggle();
	
	sampler->slideSeek( pts );
}
