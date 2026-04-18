#include <QApplication>
#include <QFileInfo>
#include <QDir>
#include <QFileDialog>
#include <QFileIconProvider>
#include <QFileSystemModel>
#include <QMessageBox>
#include <QStyle>

#include "renderingdialog.h"

namespace {

class SimpleFileIconProvider : public QFileIconProvider
{
public:
	QIcon icon( const QFileInfo &info ) const override
	{
		if ( info.isDir() )
			return QApplication::style()->standardIcon( QStyle::SP_DirIcon );
		return QApplication::style()->standardIcon( QStyle::SP_FileIcon );
	}
	QIcon icon( IconType type ) const override
	{
		switch ( type ) {
			case Folder:
				return QApplication::style()->standardIcon( QStyle::SP_DirIcon );
			case File:
			default:
				return QApplication::style()->standardIcon( QStyle::SP_FileIcon );
		}
	}
};

void configureFileDialog( QFileDialog &dialog )
{
	dialog.setOption( QFileDialog::DontUseNativeDialog, true );
	dialog.setOption( QFileDialog::DontUseCustomDirectoryIcons, true );
	dialog.setViewMode( QFileDialog::Detail );
	QFileSystemModel *model = dialog.findChild<QFileSystemModel*>();
	if ( model )
		model->setIconProvider( new SimpleFileIconProvider );
}

}



RenderingDialog::RenderingDialog( QWidget *parent, Profile prof, double playhead, 
								double sceneLen, MQueue<Frame*> *af, MQueue<Frame*> *vf )
	: QDialog( parent ),
	playheadPts( playhead ),
	encodeStartPts( 0 ),
	encodeLength( sceneLen ),
	timelineLength( sceneLen ),
	profile( prof ),
	encoderRunning( false )
{
	setupUi( this );
	cancelBtn->hide();
	
	widthSpin->setRange( 64, MAXPROJECTWIDTH );
	widthSpin->setSingleStep( 2 );
	heightSpin->setRange( 64, MAXPROJECTHEIGHT );
	heightSpin->setSingleStep( 2 );

	widthSpin->setValue(prof.getVideoWidth());
	heightSpin->setValue(prof.getVideoHeight());
	
	h264RadBtn->setChecked( true );
	timelineRadBtn->setChecked(true);
	cancelBtn->setFocus();
	
	FFmpegCommon::getGlobalInstance()->initFFmpeg();
	hevcCodecCb->addItems(FFmpegCommon::getGlobalInstance()->getHevcCodecs());
	h264CodecCb->addItems(FFmpegCommon::getGlobalInstance()->getH264Codecs());

	videoCodecSelected(0);
	
	out = new OutputFF( vf, af );
	connect( out, SIGNAL(finished()), this, SLOT(outputFinished()) );
	connect( out, SIGNAL(showFrame(Frame*)), this, SLOT(frameEncoded(Frame*)) );
	
	connect( openBtn, SIGNAL(clicked()), this, SLOT(openFile()) );
	connect( renderBtn, SIGNAL(clicked()), this, SLOT(startRender()) );
	connect( cancelBtn, SIGNAL(clicked()), this, SLOT(canceled()) );
	
	connect( heightSpin, SIGNAL(valueChanged(int)), this, SLOT(heightChanged(int)) );
	
	connect( buttonGroup, SIGNAL(buttonClicked(int)), this, SLOT(videoCodecSelected(int)) );
}



RenderingDialog::~RenderingDialog()
{
	delete out;
}



void RenderingDialog::videoCodecSelected(int id)
{
	Q_UNUSED(id);
	double brRatio = 1.0;
	if (mpeg2RadBtn->isChecked())
		brRatio = 2.0;
	else if (hevcRadBtn->isChecked())
		brRatio = 0.5;
	videoRateSpin->setValue(qRound(0.12 * brRatio * widthSpin->value() * heightSpin->value() * profile.getVideoFrameRate() / 1000000));
}



void RenderingDialog::heightChanged( int val )
{
	if (val % 2) {
		++val;
	}
	
	int w = (double)profile.getVideoWidth() * (double)val / (double)profile.getVideoHeight();
	if (w % 2) {
		++w;
	}
	widthSpin->setValue(w);
	heightSpin->setValue(val);
	
	videoCodecSelected(0);
}



void RenderingDialog::openFile()
{
	QFileDialog dialog( this, tr("Render to file") );
	dialog.setAcceptMode( QFileDialog::AcceptSave );
	configureFileDialog( dialog );
	if ( dialog.exec() != QDialog::Accepted )
		return;
	QString file = dialog.selectedFiles().value( 0 );
	if ( !file.isEmpty() ) {
		QFileInfo fi( file );
		QString suffix = fi.suffix();
		QString path = fi.absoluteFilePath();
		if ( !suffix.isEmpty() ) {
			path.truncate( path.length() - suffix.length() - 1 );
		}
		filenameLE->setText( path );
	}
}



void RenderingDialog::startRender()
{	
	double endPts = timelineLength - profile.getVideoFrameDuration();
	if ( playheadRadBtn->isChecked() ) {
		double end = playheadPts + (double)durationSpin->value() * MICROSECOND;
		if ( end < endPts )
			endPts = end;
	}
	endPts -= profile.getVideoFrameDuration() / 2.0;
	
	encodeLength = timelineLength;
	encodeStartPts = 0;
	if ( playheadRadBtn->isChecked() || playheadToEndRadBtn->isChecked() ) {
		encodeLength = endPts + profile.getVideoFrameDuration() * 3.0 / 2.0;
		encodeLength -= playheadPts;
		encodeStartPts = playheadPts;
	}
	
	QFileInfo fi( filenameLE->text() );
	QString suffix = fi.suffix();
	QString filePath = fi.absoluteFilePath();
	if ( !suffix.isEmpty() ) {
		filePath.truncate( filePath.length() - suffix.length() - 1 );
	}
	if ( !fi.dir().exists() || fi.fileName().isEmpty() ) {
		QMessageBox::warning( this, tr("Error"), tr("Invalid file name.") );
		return;
	}

	int vcodec = OutputFF::VCODEC_H264;
	QString vcodecName = h264CodecCb->currentText();
	if (mpeg2RadBtn->isChecked()) {
		vcodec = OutputFF::VCODEC_MPEG2;
		vcodecName = "";
		filePath += ".mpg";
	}
	else if (hevcRadBtn->isChecked()) {
		vcodec = OutputFF::VCODEC_HEVC;
		vcodecName = hevcCodecCb->currentText();
		filePath += ".mkv";
	}
	else {
		filePath += ".mp4";
	}

	QFileInfo fi2( filePath );
	if ( fi2.exists() && QMessageBox::Yes != QMessageBox::question( this, tr("Warning"),
		tr("The file %1 exists, do you want to overwrite it?").arg(fi2.fileName()), QMessageBox::Yes | QMessageBox::No ) )
	{
		return;
	}
	
	
	Profile p = profile;
	p.setVideoWidth(widthSpin->value());
	p.setVideoHeight(heightSpin->value());

	if ( !out->init( filePath, p, videoRateSpin->value(), vcodec, vcodecName, endPts ) ) {
		QMessageBox::warning( this, tr("Error"), tr("Could not setup encoder.") );
		return;
	}
	
	encoderRunning = true;
	enableUI( false );
	eta.start();
	QSize outputSize = QSize(0, 0);
	if (profile.getVideoHeight() != heightSpin->value()) {
		outputSize = QSize(widthSpin->value(), heightSpin->value());
	}
	emit renderStarted( encodeStartPts, outputSize );
}



void RenderingDialog::frameEncoded( Frame *f )
{
	double prc = (f->pts() - encodeStartPts) * 100.0 / encodeLength;
	double elapsed = eta.elapsed();
	double still = (elapsed * 100.0 / prc) - elapsed;
	QString s = tr("Remaining time:");
	s += "  " + QTime( 0, 0, 0 ).addMSecs( still ).toString("hh:mm:ss");
	etaLab->setText( s );
	progressBar->setValue( prc );
	emit showFrame( f );
}



void RenderingDialog::timelineReady()
{
	out->startEncode();
}



void RenderingDialog::outputFinished()
{
	emit renderFinished( playheadPts );
	enableUI( true );
	encoderRunning = false;
	etaLab->setText("");
}



void RenderingDialog::canceled()
{
	if ( QMessageBox::Yes != QMessageBox::question( this, tr("Warning"),
		tr("Do you really want to cancel?"), QMessageBox::Yes | QMessageBox::No ) )
	{
		return;
	}

	if ( !encoderRunning )
		done( QDialog::Rejected );
	else {
		out->cancel();
		encoderRunning = false;
	}
}



void RenderingDialog::enableUI( bool b )
{
	h264RadBtn->setEnabled( b );
	mpeg2RadBtn->setEnabled( b );
	hevcRadBtn->setEnabled( b );
	filenameLE->setEnabled( b );
	timelineRadBtn->setEnabled( b );
	playheadRadBtn->setEnabled( b );
	playheadToEndRadBtn->setEnabled( b );
	durationSpin->setEnabled( b );
	renderBtn->setEnabled( b );
	openBtn->setEnabled( b );
	videoRateSpin->setEnabled( b );
	progressBar->setValue( 0 );
	heightSpin->setEnabled( b );
	hevcCodecCb->setEnabled( b );
	h264CodecCb->setEnabled( b );
	if (b) {
		cancelBtn->hide();
	}
	else {
		cancelBtn->show();
	}
}



void RenderingDialog::done( int r )
{
	if ( encoderRunning )
		return;
	
	QDialog::done( r );
}
