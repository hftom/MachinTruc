#ifndef ADDCLIPSDIALOG_H
#define ADDCLIPSDIALOG_H

#include "ui_addclips.h"



class AddClipsSettings
{
public:
	QString transition;
	int imageDuration;
	int transitionDuration;
	bool panAndZoom;
};



class AddClipsDialog : public QDialog, protected Ui::AddClipsDlg
{
	Q_OBJECT
public:
	AddClipsDialog( QWidget *parent );
	AddClipsSettings getSettings();
};

#endif // ADDCLIPSDIALOG_H
