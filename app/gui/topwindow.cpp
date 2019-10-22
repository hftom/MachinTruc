#include <QMessageBox>
#include <QFileDialog>
#include <QShortcut>

#include "engine/util.h"
#include "gui/topwindow.h"
#include "renderingdialog.h"
#include "projectprofiledialog.h"

#define VIDEOCLEARDELAY 200
#define AUTORECOVERY "autorecovery.mct"



TopWindow::TopWindow()
	: openSourcesCounter( 0 ),
	projectLoader( NULL ),
	thumbnailer( new Thumbnailer() )
{
	setupUi( this );

	qRegisterMetaType<OVDUpdateMessage>();

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
	undoStack.setUndoLimit(100);
	timeline = new Timeline( this, &undoStack );

	timelineView = new TimelineGraphicsView( 0 );
	timelineView->setFocusPolicy( Qt::NoFocus );
	timelineView->setAlignment( Qt::AlignLeft | Qt::AlignTop );
	timelineView->setScene( timeline );
	timelineView->setAcceptDrops( true );
	timelineStackedWidget->addWidget( timelineView );

	connect( this, SIGNAL(setCursorPos(double,bool)), timeline, SLOT(setCursorPos(double,bool)) );
	connect( timeline, SIGNAL(seekTo(double)), this, SLOT(timelineSeek(double)) );
	connect( timeline, SIGNAL(ensureVisible(const QGraphicsItem*)), this, SLOT(ensureVisible(const QGraphicsItem*)) );
	connect( timeline, SIGNAL(centerOn(const QGraphicsItem*)), this, SLOT(centerOn(const QGraphicsItem*)) );
	connect( timeline, SIGNAL(updateFrame()), sampler, SLOT(updateFrame()) );
	connect( timeline, SIGNAL(clipAddedToTimeline(Profile)), this, SLOT(clipAddedToTimeline(Profile)) );
	connect( timeline, SIGNAL(trackRequest(bool,int)), this, SLOT(trackRequest(bool,int)) );
	connect( timelineView, SIGNAL(sizeChanged(const QSize&)), timeline, SLOT(viewSizeChanged(const QSize&)) );
	connect( timelineView, SIGNAL(viewMouseMove(QPointF)), timeline, SLOT(viewMouseMove(QPointF)) );
	connect( timelineView, SIGNAL(viewMouseLeave()), timeline, SLOT(viewMouseLeave()) );

	animEditor = new AnimEditor( 0 );
	connect( this, SIGNAL(setCursorPos(double,bool)), animEditor, SLOT(setCursorPos(double,bool)) );
	connect( animEditor, SIGNAL(quitEditor()), this, SLOT(quitEditor()) );
	connect( animEditor, SIGNAL(updateFrame()), sampler, SLOT(updateFrame()) );
	timelineStackedWidget->addWidget( animEditor );

	sourcePage = new ProjectSourcesPage( sampler );
	connect( sourcePage, SIGNAL(sourceActivated()), this, SLOT(sourceActivated()) );
	connect( sourcePage, SIGNAL(openSourcesBtnClicked()), this, SLOT(openSources()) );
	connect( sourcePage, SIGNAL(openBlankBtnClicked()), this, SLOT(openBlank()) );
	connect( sourcePage, SIGNAL(addSelectionToTimeline()), timeline, SLOT(addSelectionToTimeline()) );

	fxPage = new FxPage();
	connect( animEditor, SIGNAL(ovdValueChanged(ParameterWidget*)), fxPage, SIGNAL(ovdValueChanged(ParameterWidget*)) );
	connect( timeline, SIGNAL(clipSelected(ClipViewItem*)), fxPage, SLOT(clipSelected(ClipViewItem*)) );
	connect( timeline, SIGNAL(showEffects()), fxToolButton, SLOT(click()) );
	connect( fxPage, SIGNAL(filterDeleted(Clip*,QSharedPointer<Filter>)),
			 timeline, SLOT(filterDeleted(Clip*,QSharedPointer<Filter>)) );
	connect( fxPage, SIGNAL(filterDeleted(Clip*,QSharedPointer<Filter>)),
			 animEditor, SLOT(filterDeleted(Clip*,QSharedPointer<Filter>)) );
	connect( fxPage, SIGNAL(filterAdded(ClipViewItem*,QString,int)), timeline, SLOT(addFilter(ClipViewItem*,QString,int)) );
	connect( fxPage, SIGNAL(filterReordered(Clip*,bool,int,int)), timeline, SLOT(filterReordered(Clip*,bool,int,int)) );
	connect( fxPage, SIGNAL(paramUndoCommand(QSharedPointer<Filter>,Parameter*,QVariant,QVariant)),
			 timeline, SLOT(paramUndoCommand(QSharedPointer<Filter>,Parameter*,QVariant,QVariant)) );
	connect( fxPage, SIGNAL(updateFrame()), sampler, SLOT(updateFrame()) );
	connect( fxPage, SIGNAL(editAnimation(FilterWidget*,ParameterWidget*,Parameter*)),
			 this, SLOT(editAnimation(FilterWidget*,ParameterWidget*,Parameter*)) );
	connect( fxPage, SIGNAL(showEffect(bool,int)), timeline, SLOT(showEffect(bool,int)) );
	connect( fxPage, SIGNAL(currentFilterChanged(int)), this, SLOT(hideAnimEditor(int)) );
	connect( fxPage, SIGNAL(compileShaderRequest(ThumbRequest)), this, SLOT(clipThumbRequest(ThumbRequest)) );
	connect( fxPage, SIGNAL(filterCopy(QSharedPointer<Filter>,bool)), this, SLOT(filterCopy(QSharedPointer<Filter>,bool)) );

	fxSettingsPage = new FxSettingsPage();
	connect( timeline, SIGNAL(clipSelected(ClipViewItem*)), fxSettingsPage, SLOT(clipSelected(ClipViewItem*)) );
	connect( timeline, SIGNAL(showTransition()), fxSettingsToolButton, SLOT(click()) );
	connect( fxSettingsPage, SIGNAL(updateFrame()), sampler, SLOT(updateFrame()) );
	connect( fxSettingsPage, SIGNAL(paramUndoCommand(QSharedPointer<Filter>,Parameter*,QVariant,QVariant)),
			 timeline, SLOT(paramUndoCommand(QSharedPointer<Filter>,Parameter*,QVariant,QVariant)) );
	connect( fxSettingsPage, SIGNAL(transitionChanged(Clip*,QString,bool)), timeline, SLOT(transitionChanged(Clip*,QString,bool)) );

	stackedWidget->addWidget( sourcePage );
	stackedWidget->addWidget( fxPage );
	stackedWidget->addWidget( fxSettingsPage );
	connect( stackedWidget, SIGNAL(currentChanged(int)), this, SLOT(hideAnimEditor(int)) );

	QHBoxLayout *layout = new QHBoxLayout;
	layout->setContentsMargins( 0, 0, 0, 0 );
	vw = new VideoWidget( glWidget );
	layout->addWidget( vw );
	glWidget->setLayout( layout );

	connect( vw, SIGNAL(ovdUpdateSignal(QList<OVDUpdateMessage>)), animEditor, SLOT(ovdUpdate(QList<OVDUpdateMessage>)) );
	connect( vw, SIGNAL(playPause()), this, SLOT(videoPlayPause()) );
	connect( vw, SIGNAL(wheelSeek(int)), sampler, SLOT(wheelSeek(int)) );
	connect( vw, SIGNAL(newSharedContext(QGLWidget*)), sampler, SLOT(setSharedContext(QGLWidget*)) );
	connect( vw, SIGNAL(newFencesContext(QGLWidget*)), sampler, SLOT(setFencesContext(QGLWidget*)) );
	connect( vw, SIGNAL(newThumbContext(QGLWidget*)), this, SLOT(setThumbContext(QGLWidget*)) );
	connect( sampler->getMetronom(), SIGNAL(newFrame(Frame*)), vw, SLOT(showFrame(Frame*)) );
	connect( sampler->getMetronom(), SIGNAL(currentFramePts(double)), this, SLOT(currentFramePts(double)) );
	connect( vw, SIGNAL(frameShown(Frame*)), sampler->getMetronom(), SLOT(setLastFrame(Frame*)) );
	connect( sampler, SIGNAL(newFrame(Frame*)), vw, SLOT(showFrame(Frame*)) );
	connect( sampler, SIGNAL(paused(bool)), this, SLOT(composerPaused(bool)) );
	connect( sampler, SIGNAL(modeSwitched()), this, SLOT(modeSwitched()) );

	connect( sampler->getMetronom(), SIGNAL(osdMessage(const QString&,int)), vw, SLOT(showOSDMessage(const QString&,int)) );
	connect( sampler->getMetronom(), SIGNAL(osdTimer(bool)), vw, SLOT(showOSDTimer(bool)) );
	connect( this, SIGNAL(startOSDTimer(bool)), vw, SLOT(showOSDTimer(bool)) );

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

	//HACK: add actions to this widget to get shortcuts working with ubuntu's appmenu-qt5
	addAction( actionNewProject ); 				connect( actionNewProject, SIGNAL(triggered()), this, SLOT(newProject()) );
	addAction( actionSave );					connect( actionSave, SIGNAL(triggered()), this, SLOT(saveProject()) );
	addAction( actionOpen );					connect( actionOpen, SIGNAL(triggered()), this, SLOT(openProject()) );
	addAction( actionPlayPause );				connect( actionPlayPause, SIGNAL(triggered()), this, SLOT(videoPlayPause()) );
	addAction( actionForward );					connect( actionForward, SIGNAL(triggered()), this, SLOT(playForward()) );
	addAction( actionBackward );				connect( actionBackward, SIGNAL(triggered()), this, SLOT(playBackward()) );
	addAction( actionFrameByFrame );			connect( actionFrameByFrame, SIGNAL(triggered()), this, SLOT(seekNext()) );
	addAction( actionFrameByFrameBackward );	connect( actionFrameByFrameBackward, SIGNAL(triggered()), this, SLOT(seekPrevious()) );
	addAction( actionNextEdge );				connect( actionNextEdge, SIGNAL(triggered()), timeline, SLOT(nextEdge()) );
	addAction( actionPreviousEdge );			connect( actionPreviousEdge, SIGNAL(triggered()), timeline, SLOT(previousEdge()) );
	addAction( actionSlower );					connect( actionSlower, SIGNAL(triggered()), this, SLOT(playSlower()) );
	addAction( actionZoomIn );					connect( actionZoomIn, SIGNAL(triggered()), this, SLOT(zoomIn()) );
	addAction( actionZoomOut );					connect( actionZoomOut, SIGNAL(triggered()), this, SLOT(zoomOut()) );
	addAction( actionCopy );					connect( actionCopy, SIGNAL(triggered()), this, SLOT(editCopy()) );
	addAction( actionCut );						connect( actionCut, SIGNAL(triggered()), this, SLOT(editCut()) );
	addAction( actionPaste );					connect( actionPaste, SIGNAL(triggered()), this, SLOT(editPaste()) );
	addAction( actionSelectAll );				connect( actionSelectAll, SIGNAL(triggered()), this, SLOT(selectAll()) );
	addAction( actionSplitCurrentClip );		connect( actionSplitCurrentClip, SIGNAL(triggered()), timeline, SLOT(splitCurrentClip()) );

	connect( actionProjectSettings, SIGNAL(triggered()), this, SLOT(menuProjectSettings()) );
	connect( actionMoveMulti, SIGNAL(triggered()), this, SLOT(moveMulti()) );
	connect( actionSaveImage, SIGNAL(triggered()), vw, SLOT(shot()) );
	connect( actionRenderToFile, SIGNAL(triggered()), this, SLOT(renderDialog()) );

	clipboard = new ClipBoard(actionCopy, actionCut, actionPaste);
	connect( timeline, SIGNAL(clipSelected(ClipViewItem*)), clipboard, SLOT(clipSelected(ClipViewItem*)) );

	QAction *act = undoStack.createUndoAction(this);
	act->setIcon( QIcon(":/toolbar/icons/edit-undo.png") );
	act->setShortcut(QKeySequence(QKeySequence::Undo));
	undoToolButton->setDefaultAction( act );
	menuTimeline->insertAction( actionCopy, act );
	act = undoStack.createRedoAction(this);
	act->setIcon( QIcon(":/toolbar/icons/edit-redo.png") );
	act->setShortcut(QKeySequence(QKeySequence::Redo));
	redoToolButton->setDefaultAction( act );
	menuTimeline->insertAction( actionCopy, act );
	menuTimeline->insertSeparator( actionCopy );

	new QShortcut( QKeySequence( "Ctrl+M" ), this, SLOT(showMemoryInfo()) );
	new QShortcut( QKeySequence( "Space" ), this, SLOT(videoPlayPause()) );

	timeline->setScene( sampler->getCurrentScene() );

	appConfig.beginGroup("MainWindowGeometry");
	restoreGeometry( appConfig.value("mainWindow").toByteArray() );
	videoSplitter->restoreState( appConfig.value("videoSplitter").toByteArray() );
	timelineSplitter->restoreState( appConfig.value("timelineSplitter").toByteArray() );
	//sourcePage->getSplitter()->restoreState( appConfig.value("sourceSplitter").toByteArray() );
	fxPage->getVideoSplitter()->restoreState( appConfig.value("fxVideoSplitter").toByteArray() );
	fxPage->getVideoGraphSplitter()->restoreState( appConfig.value("fxVideoGraphSplitter").toByteArray() );
	fxPage->getAudioSplitter()->restoreState( appConfig.value("fxAudioSplitter").toByteArray() );
	fxPage->getAudioGraphSplitter()->restoreState( appConfig.value("fxAudioGraphSplitter").toByteArray() );
	appConfig.endGroup();
	appConfig.beginGroup("Paths");
	openSourcesCurrentDir = appConfig.value("openSourcesCurrentDir").toString();
	openProjectCurrentDir = appConfig.value("openProjectCurrentDir").toString();
	appConfig.endGroup();

	QDir dir = QDir::home();
	dir.cd(MACHINTRUC_DIR);
	QString backup = dir.filePath(AUTORECOVERY);
	if (QFile::exists(backup)) {
		QFileInfo info(backup);
		QDateTime lastModified = info.lastModified();
		if (lastModified.secsTo(QDateTime::currentDateTime()) <= 3600 * 24) {
			QTimer::singleShot(1, this, SLOT(loadBackup()));
		}
	}

	connect(&backupTimer, SIGNAL(timeout()), this, SLOT(doBackup()));
	backupTimer.start(300000);
}



void TopWindow::selectAll()
{
	if ( timelineStackedWidget->currentIndex() == 0 )
		timeline->selectAll();
}



void TopWindow::filterCopy(QSharedPointer<Filter> f, bool audio)
{
	clipboard->copyFilter(f, audio);
}



void TopWindow::editPaste()
{
	if ( timelineStackedWidget->currentIndex() == 0 )
		timeline->editPaste(clipboard);
}



void TopWindow::editCopy()
{
	if ( timelineStackedWidget->currentIndex() == 0 )
		timeline->editCopy(clipboard);
}



void TopWindow::editCut()
{
	if ( timelineStackedWidget->currentIndex() == 0 )
		timeline->editCut(clipboard);
}



void TopWindow::moveMulti()
{
	QMessageBox::information(this, tr("Advice"),
							 tr("Hold down the SHIFT key before dragging a clip to also move all the clips on its right side."));
}



bool TopWindow::saveAndContinue() {
	if (!undoStack.isClean()) {
		int ret = QMessageBox::question(this, tr("Unsaved changes"),
										tr("This project has been modified.\nDo you want to save your changes?"),
										QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel, QMessageBox::Yes);
		if (ret == QMessageBox::Yes) {
			saveProject();
		}
		else if (ret == QMessageBox::Cancel) {
			return false;
		}
	}
	return true;
}



void TopWindow::loadBackup()
{
	QDir dir = QDir::home();
	dir.cd(MACHINTRUC_DIR);
	QString backup = dir.filePath(AUTORECOVERY);
	QString backupFilename = "";

	if (loadProject( backup, backupFilename )) {
		if (backupFilename != "") {
			QFileInfo fi( backupFilename );
			openProjectCurrentDir = fi.absolutePath();
			currentProjectFile = fi.absoluteFilePath();
		}

		QMessageBox msgBox;
		msgBox.setText( tr("Auto Recovery found and loaded.") );
		msgBox.exec();
	}
	else {
		QMessageBox msgBox;
		msgBox.setText( tr("Auto Recovery failed.\nThe file seems corrupted.") );
		msgBox.exec();
	}
}



void TopWindow::doBackup()
{
	QDir dir = QDir::home();
	if ( !dir.cd( MACHINTRUC_DIR ) ) {
		if ( !dir.mkdir( MACHINTRUC_DIR ) ) {
			qDebug() << "Can't create" << MACHINTRUC_DIR << "directory.";
			return;
		}
		if ( !dir.cd( MACHINTRUC_DIR ) ) {
			qDebug() << "Can't enter" << MACHINTRUC_DIR << "directory.";
			return;
		}
	}

	ProjectFile xml;
	QList<Source*> sources = sourcePage->getAllSources();
	if (sources.count()) {
		xml.saveProject( sources, sampler, dir.filePath(AUTORECOVERY), currentProjectFile );
	}
}



void TopWindow::removeBackup()
{
	QDir dir = QDir::home();
	dir.cd(MACHINTRUC_DIR);
	QFile::remove(dir.filePath(AUTORECOVERY));
}



bool TopWindow::ignoreBackgroundJobsRunning()
{
	if ( StabilizeCollection::getGlobalInstance()->hasRunningJobs() ) {
		int ret = QMessageBox::question(this, tr("Running jobs"),
										tr("Some clips may not be stabilized yet.\nContinue anyway?"),
										QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
		if (ret == QMessageBox::No) {
			return false;
		}
	}
	return true;
}



void TopWindow::closeEvent( QCloseEvent *event )
{
	if (!ignoreBackgroundJobsRunning() || !saveAndContinue()) {
		event->ignore();
		return;
	}

	appConfig.beginGroup( "MainWindowGeometry" );
	appConfig.setValue( "mainWindow", saveGeometry() );
	appConfig.setValue( "videoSplitter", videoSplitter->saveState() );
	appConfig.setValue( "timelineSplitter", timelineSplitter->saveState() );
	//appConfig.setValue( "sourceSplitter", sourcePage->getSplitter()->saveState() );
	appConfig.setValue( "fxVideoSplitter", fxPage->getVideoSplitter()->saveState() );
	appConfig.setValue( "fxVideoGraphSplitter", fxPage->getVideoGraphSplitter()->saveState() );
	appConfig.setValue( "fxAudioSplitter", fxPage->getAudioSplitter()->saveState() );
	appConfig.setValue( "fxAudioGraphSplitter", fxPage->getAudioGraphSplitter()->saveState() );
	appConfig.endGroup();
	appConfig.beginGroup("Paths");
	appConfig.setValue("openSourcesCurrentDir", openSourcesCurrentDir);
	appConfig.setValue("openProjectCurrentDir", openProjectCurrentDir);
	appConfig.endGroup();

	removeBackup();
}



void TopWindow::keyPressEvent( QKeyEvent *event )
{
	if ( event->key() == Qt::Key_Control )
		vw->controlKeyPressed( true );
	QMainWindow::keyPressEvent( event );
}



void TopWindow::keyReleaseEvent( QKeyEvent *event )
{
	if ( event->key() == Qt::Key_Control )
		vw->controlKeyPressed( false );
	QMainWindow::keyPressEvent( event );
}



void TopWindow::zoomIn()
{
	if ( timelineStackedWidget->currentIndex() == 0 )
		timeline->zoomInOut( true );
}



void TopWindow::zoomOut()
{
	if ( timelineStackedWidget->currentIndex() == 0 )
		timeline->zoomInOut( false );
}



void TopWindow::showMemoryInfo()
{
	MemDialog *d = new MemDialog( this );
	d->setModal( false );
	d->show();
}



void TopWindow::renderDialog()
{
	if ( !sampler->currentTimelineSceneDuration() ) {
		QMessageBox::warning( this, tr("Sorry"), tr("The timeline is empty.") );
		return;
	}

	if ( !ignoreBackgroundJobsRunning() ) {
		return;
	}

	double playhead = 0;
	Frame *f = sampler->getMetronom()->getLastFrame();
	if ( f && !sampler->previewMode() )
		playhead = f->pts();
	else
		playhead = sampler->getCurrentScene()->currentPTS;

	RenderingDialog *dlg = new RenderingDialog( this,
								sampler->getCurrentScene()->getProfile(),
								playhead,
								sampler->currentTimelineSceneDuration(),
								&sampler->getMetronom()->audioFrames,
								&sampler->getMetronom()->encodeVideoFrames );

	connect( dlg, SIGNAL(renderStarted(double, QSize)), this, SLOT(renderStart(double, QSize)) );
	connect( dlg, SIGNAL(renderFinished(double)), this, SLOT(renderFinished(double)) );
	connect( dlg, SIGNAL(showFrame(Frame*)), vw, SLOT(showFrame(Frame*)) );
	connect( this, SIGNAL(timelineReadyForEncode()), dlg, SLOT(timelineReady()) );
	dlg->exec();
	delete dlg;
}

void TopWindow::renderStart( double startPts, QSize out )
{
	sampler->setOutputResize(out);
	timelineSeek( startPts );
	vw->clear();
	sampler->getMetronom()->setRenderMode( true );
	playPause( true );
	emit timelineReadyForEncode();
}

void TopWindow::renderFinished( double pts )
{
	timelineSeek( pts );
	sampler->getMetronom()->setRenderMode( false );
	sampler->setOutputResize(QSize(0, 0));
	timelineSeek( pts );
}



void TopWindow::trackRequest( bool rm, int index )
{
	if ( rm ) {
		if ( !sampler->trackRequest( rm, index ) ) {
			QMessageBox::warning( this, tr("Error"), tr("Only empty track can be deleted.") );
		}
		else {
			timeline->trackRemoved( index );
		}
	}
	else {
		if ( !sampler->trackRequest( rm, index ) ) {
			QMessageBox::warning( this, tr("Error"), tr("Track could not be added.") );
		}
		else {
			timeline->trackAdded( index );
		}
	}
}



void TopWindow::timelineTrackAddRemove( int index, bool remove )
{
	sampler->trackRequest( remove, index );
}



QList<Source*> TopWindow::getAllSources()
{
	return sourcePage->getAllSources();
}



void TopWindow::clipAddedToTimeline( Profile prof )
{
	tempProfile = prof;
	QTimer::singleShot( 1, this, SLOT(projectSettings()) );
}



void TopWindow::newProject()
{
	if ( openSourcesCounter != 0 ) {
		QMessageBox msgBox;
		msgBox.setText( tr("Files are being loaded.\nWait until the operation has completed.") );
		msgBox.exec();
		return;
	}

	if (!saveAndContinue()) {
		return;
	}

	tempProfile = Profile();
	showProjectClipsPage();
	sampler->newProject( tempProfile );
	timeline->setScene( sampler->getCurrentScene() );
	sourcePage->clearAllSources();
	timelineSeek( 0 );
	QTimer::singleShot( VIDEOCLEARDELAY, vw, SLOT(clear()) );
	currentProjectFile = "";
	undoStack.clear();
	clipboard->reset();
}



void TopWindow::menuProjectSettings()
{
	tempProfile = sampler->getCurrentScene()->getProfile();
	int warn = WARNNO;
	if ( !sampler->isProjectEmpty() )
		warn = WARNCHANGE;
	projectSettings( warn );
}



void TopWindow::projectSettings( int warn )
{
	ProjectProfileDialog dlg( this, tempProfile, warn );
	dlg.exec();
	if ( dlg.result() == QDialog::Accepted ) {
		if (warn == WARNCHANGE) {
			if (!saveAndContinue()) {
				return;
			}
			undoStack.clear();
		}

		if ( !sampler->setProfile( dlg.getCurrentProfile() ) )
			QMessageBox::warning( this, tr("Error"), tr("Somme errors occured while changing profile.\nCheck clips alignment.") );
		timeline->setScene( sampler->getCurrentScene() );

	}
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



void TopWindow::hideAnimEditor( int )
{
	if ( timelineStackedWidget->currentIndex() != 0 ) {
		animEditor->setCurrentParam( NULL, NULL, NULL );
		timelineStackedWidget->setCurrentIndex( 0 );
	}
}



void TopWindow::modeSwitched()
{
	if ( sourcePage->hasActiveSource() )
		sampler->setSource( sourcePage->sourceActiveSource(), sourcePage->currentPtsActiveSource() );
}



void TopWindow::setInPoint()
{
	// ATTENTION preview pts is now in timeline range (start at 0)
	if ( sourcePage->hasActiveSource() ) {
		//double pts;
		//QImage img = vw->getThumb( THUMBHEIGHT, &pts, false );
		//if ( !img.isNull() )
			//activeSource->setInPoint( img, pts );
	}
}



void TopWindow::setOutPoint()
{
	// ATTENTION preview pts is now in timeline range (start at 0)
	if ( sourcePage->hasActiveSource() ) {
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
	if ( mime == MIMETYPESOURCE ) {
		Source *source = sourcePage->getSource( index, filename );
		start = source->getProfile().getStreamStartTime();
		len = source->getProfile().getStreamDuration();
		return source;
	}
	return NULL;
}



QList<Source*> TopWindow:: getSelectedSources()
{
	return sourcePage->getSelectedSources();
}



void TopWindow::ensureVisible( const QGraphicsItem *it )
{
	//if ( playToolButton->isChecked() )
		timelineView->ensureVisible( it, 100, 100 );
}



void TopWindow::centerOn( const QGraphicsItem *it )
{
	timelineView->centerOn( it );
}



void TopWindow::sourceActivated()
{
	if ( sourcePage->hasActiveSource() ) {
		if ( switchButton->isChecked() )
			switchButton->toggle();
		else
			sampler->setSource( sourcePage->sourceActiveSource(), sourcePage->currentPtsActiveSource() );
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


void TopWindow::currentFramePts( double d )
{
	if ( !switchButton->isChecked() ) {
		if ( !sourcePage->hasActiveSource() )
			return;
		sourcePage->activeSourceSetCurrentPts( d );
	}
	else {
		emit setCursorPos( d, sampler->getMetronom()->isPlaying() );
	}

	if ( !seekSlider->isButtonDown() ) {
		d *= seekSlider->maximum() / sampler->currentSceneDuration();
		seekSlider->blockSignals( true );
		seekSlider->setValue( d );
		seekSlider->blockSignals( false );
	}
}



void TopWindow::composerPaused( bool b )
{
	// avoid toggled() to be emitted
	playToolButton->blockSignals( true );
	playToolButton->setChecked( !b );
	playToolButton->blockSignals( false );
}



void TopWindow::playPause( bool playing )
{
	sampler->play( playing );
}



void TopWindow::videoPlayPause()
{
	playToolButton->toggle();
}



void TopWindow::playForward()
{
	if ( sampler->play( true ) )
		composerPaused( false );
	else
		playFaster();
}



void TopWindow::playBackward()
{
	if ( sampler->play( true, true ) )
		composerPaused( false );
	else
		playFaster();
}



void TopWindow::playFaster()
{
	sampler->getMetronom()->changeSpeed( 1 );
}



void TopWindow::playSlower()
{
	sampler->getMetronom()->changeSpeed( -1 );
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
	double value = v * sampler->currentSceneDuration() / seekSlider->maximum();
	sampler->slideSeek( value );
}



void TopWindow:: timelineSeek( double pts )
{
	if ( !switchButton->isChecked() )
		switchButton->toggle();

	sampler->slideSeek( pts );
}



void TopWindow::setThumbContext( QGLWidget *context )
{
	thumbnailer->setSharedContext( context );
	connect( thumbnailer, SIGNAL(resultReady()), thumbnailer, SLOT(gotResult()) );
	connect( thumbnailer, SIGNAL(thumbReady(ThumbRequest)), this, SLOT(thumbResultReady(ThumbRequest)) );
}



void TopWindow::clipThumbRequest( ThumbRequest request )
{
	thumbnailer->pushRequest( request );
}



void TopWindow::openBlank()
{
	QString s = "Blank";
	if ( sourcePage->exists( s ) )
		duplicateOpenSources.append( s );
	else {
		if ( thumbnailer->pushRequest( ThumbRequest( s ) ) )
			++openSourcesCounter;
	}
	if ( !openSourcesCounter )
		unsupportedDuplicateMessage();
}



void TopWindow::openSources()
{
	QStringList	list = QFileDialog::getOpenFileNames( this, tr("Open files"), openSourcesCurrentDir,
		"Videos(*.3gp *.ac3 *.avi *.dv *.eac3 *.flac *.flv *.jpg *.jpeg *.m2t *.mjpg *.mkv *.mov *.mp2 *.mp3 *.mp4 *.mpa *.mpc *.mpeg *.mpg *.mts *.ogg *.png *.ts *.vob *.wav *webm *.wma *.wmv)" );

	if ( !list.isEmpty() ) {
		openSourcesCurrentDir = QFileInfo( list[0] ).absolutePath();

		for ( int i = 0; i < list.count(); ++i ) {
			if ( sourcePage->exists( list[i] ) )
				duplicateOpenSources.append( QFileInfo( list[i] ).fileName() );
			else {
				if ( thumbnailer->pushRequest( ThumbRequest( list[i] ) ) )
					++openSourcesCounter;
			}
		}
		if ( !openSourcesCounter )
			unsupportedDuplicateMessage();
	}
}



void TopWindow::unsupportedDuplicateMessage()
{
	QMessageBox msgBox;
	msgBox.setText( tr("Some files are unsupported or already part of the project.") );
	msgBox.setDetailedText( unsupportedOpenSources.join( "\n" ) + "\n" + duplicateOpenSources.join( "\n" ) );
	msgBox.exec();

	unsupportedOpenSources.clear();
	duplicateOpenSources.clear();
}



void TopWindow::thumbResultReady( ThumbRequest result )
{
	if ( result.typeOfRequest == ThumbRequest::THUMB ) {
		timeline->thumbResultReady( result );
		return;
	}

	if ( result.typeOfRequest == ThumbRequest::SHADER ) {
		ShaderEdit *edit = (ShaderEdit*)result.caller;
		if ( edit )
			edit->setCompileResult( result.filePath );
		return;
	}

	if ( projectLoader ) {
		Source *source = NULL;
		for ( int i = 0; i < projectLoader->sourcesList.count(); ++i ) {
			if ( projectLoader->sourcesList[i]->getFileName() == result.filePath ) {
				source = projectLoader->sourcesList.takeAt( i );
				break;
			}
		}

		if ( !source ) // what happens???
			return;

		if ( !result.thumb.isNull() ) {
			if ( !source->getType() )
				source->setAfter( (InputBase::InputType)result.inputType, result.profile );
			sourcePage->addSource( QPixmap::fromImage( result.thumb ), source );
		}
		else {
			for ( int k = 0; k < projectLoader->sceneList.count(); ++k ) {
				Scene *scene = projectLoader->sceneList[k];
				for ( int i = 0; i < scene->tracks.count(); ++i ) {
					Track *t = scene->tracks[i];
					for ( int j = 0; j < t->clipCount(); ++j ) {
						Clip *c = t->clipAt( j );
						if ( c->sourcePath() == source->getFileName() ) {
							t->removeClip( j-- );
							delete c;
						}
					}
				}
			}
			unsupportedOpenSources.append( source->getFileName() );
			delete source;
		}

		if ( !projectLoader->sourcesList.count() ) {
			setEnabled( true );
			sampler->setSceneList( projectLoader->sceneList );
			timeline->setScene( sampler->getCurrentScene() );
			timelineSeek( 0 );
			emit startOSDTimer( false );

			if ( projectLoader->readError ) {
				QMessageBox msgBox;
				msgBox.setText( tr("Some errors occured while reading the project file.\nCheck the timeline.") );
				msgBox.exec();
			}
			else if ( unsupportedOpenSources.count() ) {
				QMessageBox msgBox;
				msgBox.setText( tr("Some sources are unsupported or missing.\nCorresponding clips have been removed.") );
				msgBox.setDetailedText( unsupportedOpenSources.join( "\n" ) );
				msgBox.exec();
			}
			delete projectLoader;
			projectLoader = NULL;
		}
	}
	else {
		if ( !result.thumb.isNull() ) {
			Source *source = new Source( (InputBase::InputType)result.inputType, result.filePath, result.profile );
			sourcePage->addSource( QPixmap::fromImage( result.thumb ), source );
		}
		else
			unsupportedOpenSources.append( QFileInfo( result.filePath ).fileName() );

		if ( --openSourcesCounter == 0 ) {
			if ( unsupportedOpenSources.count() || duplicateOpenSources.count() )
				unsupportedDuplicateMessage();
		}
	}
}



void TopWindow::saveProject()
{
	if ( currentProjectFile.isEmpty() ) {
		QString file = QFileDialog::getSaveFileName( this, tr("Save project"),
						openProjectCurrentDir, "MachinTruc(*.mct)" );

		if ( file.isEmpty() )
			return;
		openProjectCurrentDir = QFileInfo( file ).absolutePath();
		currentProjectFile = file;
	}

	ProjectFile xml;
	xml.saveProject( sourcePage->getAllSources(), sampler, currentProjectFile );
	vw->showOSDMessage( QString( tr("Saved: %1") ).arg(currentProjectFile), 3 );
	undoStack.setClean();

	removeBackup();
}



void TopWindow::openProject()
{
	if ( openSourcesCounter != 0 ) {
		QMessageBox msgBox;
		msgBox.setText( tr("You can't load a project while files are being loaded.\nWait until the operation has completed.") );
		msgBox.exec();
		return;
	}

	if (!saveAndContinue()) {
		return;
	}

	QString file = QFileDialog::getOpenFileName( this, tr("Open project"),
						openProjectCurrentDir, "MachinTruc(*.mct)" );

	if ( file.isEmpty() )
		return;

	showProjectClipsPage();
	sampler->clearAll();
	timeline->setScene( sampler->getCurrentScene() );
	sourcePage->clearAllSources();
	undoStack.clear();
	QTimer::singleShot( VIDEOCLEARDELAY, vw, SLOT(clear()) );

	QString backup; // unused here

	if (loadProject( file, backup )) {
		QFileInfo fi( file );
		openProjectCurrentDir = fi.absolutePath();
		currentProjectFile = fi.absoluteFilePath();
	}
	else {
		QMessageBox msgBox;
		msgBox.setText( tr("Project loading failed.\nThe file seems corrupted.") );
		msgBox.exec();
	}
}



bool TopWindow::loadProject(QString filename, QString &backupFilename)
{
	bool ret = false;

	showProjectClipsPage();
	sampler->clearAll();
	timeline->setScene( sampler->getCurrentScene() );
	sourcePage->clearAllSources();
	undoStack.clear();
	QTimer::singleShot( VIDEOCLEARDELAY, vw, SLOT(clear()) );

	projectLoader = new ProjectFile();

	if ( projectLoader->loadProject( filename ) && projectLoader->sourcesList.count() ) {
		int i;
		for ( i = 0; i < projectLoader->sourcesList.count(); ++i ) {
			thumbnailer->pushRequest( ThumbRequest( projectLoader->sourcesList[i]->getFileName(), projectLoader->sourcesList[i]->getType() ) );
		}
		if ( i > 0 ) {
			setEnabled( false );
			emit startOSDTimer( true );
		}
		backupFilename = projectLoader->backupFilename;
		ret = true;
	}
	else {
		while ( projectLoader->sourcesList.count() )
			delete projectLoader->sourcesList.takeFirst();

		while ( projectLoader->sceneList.count() )
			delete projectLoader->sceneList.takeFirst();

		delete projectLoader;
		projectLoader = NULL;
	}

	return ret;
}
