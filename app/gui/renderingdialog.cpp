#include <QFileInfo>
#include <QDir>
#include <QMessageBox>
#include <QFileDialog>

#include "renderingdialog.h"



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
	
	widthSpin->setRange( 64, MAXPROJECTWIDTH );
	widthSpin->setSingleStep( 2 );
	heightSpin->setRange( 64, MAXPROJECTHEIGHT );
	heightSpin->setSingleStep( 2 );

	widthSpin->setValue(prof.getVideoWidth());
	heightSpin->setValue(prof.getVideoHeight());
	
	h264RadBtn->setChecked( true );
	playheadRadBtn->setChecked( true );
	cancelBtn->setFocus();

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
	QString file = QFileDialog::getSaveFileName( this, tr("Render to file") );
	if ( !file.isEmpty() )
		filenameLE->setText( file );
}



void RenderingDialog::startRender()
{
	QFileInfo fi( filenameLE->text() );
	if ( !fi.dir().exists() || fi.fileName().isEmpty() ) {
		QMessageBox::warning( this, tr("Error"), tr("Invalid file name.") );
		return;
	}
	if ( fi.exists() && QMessageBox::Yes != QMessageBox::question( this, tr("Warning"),
		tr("This file exists, do you want to overwrite it?"), QMessageBox::Yes | QMessageBox::No ) )
		return;
	
	QString suffix = fi.suffix();
	QString s = fi.absoluteFilePath();
	if ( !suffix.isEmpty() )
		s.truncate( s.length() - suffix.length() - 1 );
	
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
	
	int vcodec = OutputFF::VCODEC_H264;
	if (mpeg2RadBtn->isChecked()) {
		vcodec = OutputFF::VCODEC_MPEG2;
	}
	else if (hevcRadBtn->isChecked()) {
		vcodec = OutputFF::VCODEC_HEVC;
	}
	
	Profile p = profile;
	p.setVideoWidth(widthSpin->value());
	p.setVideoHeight(heightSpin->value());

	if ( !out->init( s, p, videoRateSpin->value(), vcodec, endPts ) ) {
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
	filenameLE->setEnabled( b );
	timelineRadBtn->setEnabled( b );
	playheadRadBtn->setEnabled( b );
	playheadToEndRadBtn->setEnabled( b );
	durationSpin->setEnabled( b );
	renderBtn->setEnabled( b );
	openBtn->setEnabled( b );
	videoRateSpin->setEnabled( b );
	progressBar->setValue( 0 );
}



void RenderingDialog::done( int r )
{
	if ( encoderRunning )
		return;
	
	QDialog::done( r );
}
