#include "gui/topwindow.h"



TopWindow::TopWindow()
	: activeSource( NULL )
{
	setupUi( this );

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
	
	sampler = new Sampler();
	
	timeline = new Timeline( this );

	timelineView = new TimelineGraphicsView( 0 );
	timelineView->setFocusPolicy( Qt::NoFocus );
	timelineView->setAlignment( Qt::AlignLeft | Qt::AlignTop );
	timelineView->setScene( timeline );
	timelineView->setAcceptDrops( true );
	timelineStackedWidget->addWidget( timelineView );
	
	connect( this, SIGNAL(setCursorPos(double)), timeline, SLOT(setCursorPos(double)) );
	connect( timeline, SIGNAL(seekTo(double)), this, SLOT(timelineSeek(double)) );
	connect( timeline, SIGNAL(ensureVisible(const QGraphicsItem*)), this, SLOT(ensureVisible(const QGraphicsItem*)) );
	connect( timeline, SIGNAL(centerOn(const QGraphicsItem*)), this, SLOT(centerOn(const QGraphicsItem*)) );
	connect( timeline, SIGNAL(updateFrame()), sampler, SLOT(updateFrame()) );
	connect( timelineView, SIGNAL(sizeChanged(const QSize&)), timeline, SLOT(viewSizeChanged(const QSize&)) );
	
	animEditor = new AnimEditor( 0 );
	connect( animEditor, SIGNAL(quitEditor()), this, SLOT(quitEditor()) );
	connect( animEditor, SIGNAL(updateFrame()), sampler, SLOT(updateFrame()) );
	timelineStackedWidget->addWidget( animEditor );
	
	sourcePage = new ProjectSourcesPage( sampler );
	connect( sourcePage, SIGNAL(sourceActivated(SourceListItem*)), this, SLOT(sourceActivated(SourceListItem*)) );
	
	fxPage = new FxPage();
	connect( timeline, SIGNAL(clipSelected(Clip*)), fxPage, SLOT(clipSelected(Clip*)) );
	connect( fxPage, SIGNAL(filterDeleted(Clip*,QSharedPointer<Filter>)), timeline, SLOT(filterDeleted(Clip*,QSharedPointer<Filter>)) );
	connect( fxPage, SIGNAL(filterDeleted(Clip*,QSharedPointer<Filter>)), animEditor, SLOT(filterDeleted(Clip*,QSharedPointer<Filter>)) );
	connect( fxPage, SIGNAL(updateFrame()), sampler, SLOT(updateFrame()) );
	connect( fxPage, SIGNAL(editAnimation(FilterWidget*,ParameterWidget*,Parameter*)), this, SLOT(editAnimation(FilterWidget*,ParameterWidget*,Parameter*)) );
	
	stackedWidget->addWidget( sourcePage );
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
	connect( vw, SIGNAL(newThumbContext(QGLWidget*)), sourcePage, SLOT(setSharedContext(QGLWidget*)) );
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
	
	timeline->setScene( sampler->getScene() );

	connect( actionBlackBackground, SIGNAL(toggled(bool)), vw, SLOT(setBlackBackground(bool)) );
	connect( actionDeleteClip, SIGNAL(triggered()), timeline, SLOT(deleteClip()) );
	connect( actionSplitCurrentClip, SIGNAL(triggered()), timeline, SLOT(splitCurrentClip()) );
}



void TopWindow::editAnimation( FilterWidget* f, ParameterWidget *pw, Parameter *p )
{
	timelineStackedWidget->setCurrentIndex( 1 );
	animEditor->setCurrentParam( f, pw, p );
}



void TopWindow::quitEditor()
{
	timelineStackedWidget->setCurrentIndex( 0 );
}



void TopWindow::modeSwitched()
{
	if ( activeSource )
		sampler->setSource( activeSource->getSource(), activeSource->getCurrentPts() );
}



void TopWindow::setInPoint()
{
	if ( activeSource ) {
		//double pts;
		//QImage img = vw->getThumb( THUMBHEIGHT, &pts, false );
		//if ( !img.isNull() )
			//activeSource->setInPoint( img, pts );
	}
}



void TopWindow::setOutPoint()
{
	if ( activeSource ) {
		//double pts;
		//QImage img = vw->getThumb( THUMBHEIGHT, &pts, true );
		//if ( !img.isNull() ) {
			//activeSource->setOutPoint( img, pts );
			//sourcePage->newCut( activeSource );
		//}
	}
}



Source* TopWindow::getDroppedCut( int index, QString mime, QString filename, double &start, double &len )
{
	if ( mime == MIMETYPECUT ) {
		if ( !activeSource )
			return NULL;
		Cut *cut = activeSource->getCut( index, filename );
		if ( !cut )
			return NULL;
		start = cut->getStart();
		len = cut->getLength();
		return cut->getSource();
	}
	if ( mime == MIMETYPESOURCE ) {
		Source *source = sourcePage->getSource( index, filename );
		start = source->getProfile().getStreamStartTime();
		len = source->getProfile().getStreamDuration();
		return source;
	}
	return NULL;
}



void TopWindow::currentFramePts( double d )
{
	if ( !switchButton->isChecked() ) {
		if ( !activeSource )
			return;
		activeSource->setCurrentPts( d );
		Profile prof = activeSource->getProfile();
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
	if ( playToolButton->isChecked() )
		timelineView->ensureVisible( it, 100, 100 );
}



void TopWindow::centerOn( const QGraphicsItem *it )
{
	timelineView->centerOn( it );
}



void TopWindow::composerPaused( bool b )
{
	// avoid toggled() to be emitted
	playToolButton->blockSignals( true );
	playToolButton->setChecked( !b );
	playToolButton->blockSignals( false );
}



void TopWindow::sourceActivated( SourceListItem *item )
{
	if ( item ) {
		activeSource = item;
		if ( switchButton->isChecked() )
			switchButton->toggle();
		else
			sampler->setSource( activeSource->getSource(), activeSource->getCurrentPts() );
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
		if ( !activeSource )
			return;
		Profile prof = activeSource->getProfile();
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
