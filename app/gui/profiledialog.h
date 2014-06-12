#ifndef PROFILEDIALOG_H
#define PROFILEDIALOG_H

#include <QTime>

#include "engine/profile.h"
#include "ui_profile.h"



class ProfileDialog : public QDialog, protected Ui::ProfileDlg
{
	Q_OBJECT
public:
	ProfileDialog( QWidget *parent, QString fn, Profile p ) : QDialog( parent ) {
		setupUi( this );
		nameLab->setText( fn );
		int hours, mins, secs/*, us*/;
		secs = p.getStreamDuration() / MICROSECOND;
		mins = secs / 60;
		secs %= 60;
		hours = mins / 60;
		mins %= 60;
		timeLab->setText( QTime( hours, mins, secs ).toString("hh:mm:ss") );
		if ( p.hasVideo() ) {
			sizeLab->setText( QString("%1 x %2").arg( p.getVideoWidth() ).arg( p.getVideoHeight() ) );
			videoCodec->setText( p.getVideoCodecName() );
			fpsLab->setText( QString::number( p.getVideoFrameRate(), 'f', 2 ) );
			fullRangeBox->setChecked( p.getVideoColorFullRange() );
		}
		if ( p.hasAudio() ) {
			audioCodec->setText( p.getAudioCodecName() );
			sampleRate->setText( QString::number( p.getAudioSampleRate() ) );
			channels->setText( QString::number( p.getAudioChannels() ) );
			layoutName->setText( p.getAudioLayoutName() );
		}		
	};
};
#endif // PROFILEDIALOG_H