#ifndef FXPAGE_H
#define FXPAGE_H

#include <QWidget>

#include "ui_fxpage.h"
#include "engine/clip.h"



class FxPage : public QWidget, private Ui::StackFx
{
	Q_OBJECT
public:
	FxPage();
	
public slots:
	void clipSelected( Clip *clip );

private:
	QWidget *currentEffectsWidget;
	
signals:
	void filterDeleted( Clip*, Filter* );

};

#endif // FXPAGE_H