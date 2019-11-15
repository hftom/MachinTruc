#include "gui/addclipsdialog.h"



AddClipsDialog::AddClipsDialog( QWidget *parent ) : QDialog( parent )
{
	setupUi( this );
}



AddClipsSettings AddClipsDialog::getSettings()
{
	AddClipsSettings s;
	s.transition = crossfadeTransitionRad->isChecked() ? "crossfade" : "none";
	s.imageDuration = imageDurationSpin->value();
	s.transitionDuration = transitionDurationSpin->value();
	s.panAndZoom = panZoomCb->isChecked();
	
	return s;
}
