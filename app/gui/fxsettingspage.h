#ifndef FXSETTINGSPAGE_H
#define FXSETTINGSPAGE_H

#include <QWidget>

#include "ui_fxsettingspage.h"



class FxSettingsPage : public QWidget, private Ui::StackFxSettings
{
	Q_OBJECT
public:
	FxSettingsPage();

private slots:


private:


};
#endif // FXSETTINGSPAGE_H