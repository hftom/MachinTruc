#include "projectprofiledialog.h"


typedef struct AudioPreset_ {
	int samplerate;
	int layout;
} AudioPreset;

typedef struct VideoPreset_ {
	int width;
	int height;
	QString ratio;
	double framerate;
	int scanning;
} VideoPreset;

static const int nvpresets = 20;
static VideoPreset vpresets[ nvpresets ] = {
	{ 1920, 1080, "16:9", 23.97, 0 },
	{ 1920, 1080, "16:9", 24, 0 },
	{ 1920, 1080, "16:9", 25, 0 },
	{ 1920, 1080, "16:9", 29.97, 0 },
	{ 1920, 1080, "16:9", 30, 0 },
	{ 1920, 1080, "16:9", 50, 0 },
	{ 1920, 1080, "16:9", 59.94, 0 },
	{ 1920, 1080, "16:9", 60, 0 },
	
	{ 1280, 720, "16:9", 23.97, 0 },
	{ 1280, 720, "16:9", 24, 0 },
	{ 1280, 720, "16:9", 25, 0 },
	{ 1280, 720, "16:9", 29.97, 0 },
	{ 1280, 720, "16:9", 30, 0 },
	{ 1280, 720, "16:9", 50, 0 },
	{ 1280, 720, "16:9", 59.94, 0 },
	{ 1280, 720, "16:9", 60, 0 },
	
	{ 720, 576, "16:9", 25, 0 },
	{ 720, 576, "4:3", 25, 0 },
	{ 720, 480, "16:9", 29.97, 0 },
	{ 720, 480, "4:3", 29.97, 0 }
};



ProjectProfileDialog::ProjectProfileDialog( QWidget *parent, Profile p ) : QDialog( parent )
{
	setupUi( this );

	audioPresetLab->setEnabled( false );
	audioPresetCombo->setEnabled( false );
	layoutLab->setEnabled( false );
	layoutCombo->setEnabled( false );
	samplerateLab->setEnabled( false );
	samplerateSpinBox->setEnabled( false );

	videoPresetCombo->addItem( tr("Custom") );
	for ( int i = 0; i < nvpresets; ++i ) {
		videoPresetCombo->addItem( QString("%1x%2 - %3 - %4fps").arg(vpresets[i].width)
															.arg(vpresets[i].height)
															.arg(vpresets[i].ratio)
															.arg(vpresets[i].framerate) );
	}

	/*	if ( p.hasVideo() ) {
			sizeLab->setText( QString("%1 x %2").arg( p.getVideoWidth() ).arg( p.getVideoHeight() ) );
			videoCodec->setText( p.getVideoCodecName() );
			fpsLab->setText( QString::number( p.getVideoFrameRate(), 'f', 2 ) );
			fullRangeBox->setChecked( p.getVideoColorFullRange() );
			primCombo->insertItem( 0, p.colorPrimariesName() );
			gammaCombo->insertItem( 0, p.gammaCurveName() );
			spcCombo->insertItem( 0, p.colorSpaceName() );
			if ( p.getVideoInterlaced() ) {
				if ( p.getVideoTopFieldFirst() )
					interlaceCombo->insertItem( 0, tr("TFF") );
				else
					interlaceCombo->insertItem( 0, tr("BFF") );
			}
			else
				interlaceCombo->insertItem( 0, tr("No") );
			sarSpin->setValue( p.getVideoSAR() );
		}
	if ( p.hasAudio() ) {
		audioCodec->setText( p.getAudioCodecName() );
		sampleRate->setText( QString::number( p.getAudioSampleRate() ) );
		channels->setText( QString::number( p.getAudioChannels() ) );
		layoutName->setText( p.getAudioLayoutName() );
	}*/
}

