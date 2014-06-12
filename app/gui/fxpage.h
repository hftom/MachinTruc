#ifndef FXPAGE_H
#define FXPAGE_H

#include <QWidget>

#include "ui_fxpage.h"



class FxPage : public QWidget, private Ui::StackFx
{
	Q_OBJECT
public:
	FxPage();

private slots:


private:


};
#endif // FXPAGE_H