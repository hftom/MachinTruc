#ifndef PROJECTPROFILEDIALOG_H
#define PROJECTPROFILEDIALOG_H

#include <QTime>

#include "engine/profile.h"
#include "ui_projectprofile.h"



class ProjectProfileDialog : public QDialog, protected Ui::ProjectProfileDlg
{
	Q_OBJECT
public:
	ProjectProfileDialog( QWidget *parent, Profile &p );
	Profile getCurrentProfile();
	
private slots:
	void presetChanged( int index );

};
#endif // PROJECTPROFILEDIALOG_H
