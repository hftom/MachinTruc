#ifndef FXSETTINGSPAGE_H
#define FXSETTINGSPAGE_H

#include <QWidget>

#include "timeline/clipviewitem.h"
#include "ui_fxsettingspage.h"



class FxSettingsPage : public QWidget, private Ui::StackFxSettings
{
	Q_OBJECT
public:
	FxSettingsPage();

public slots:
	void clipSelected( ClipViewItem *clip );
	
private slots:
	void videoFilterActivated( const QString& text );
	void audioFilterActivated( const QString& text );

private:
	void setComboItems( Transition *t );
	
	QWidget *currentEffectWidget;
	QGridLayout *effectWidgetLayout;
	
	QWidget *currentEffectWidgetAudio;
	QGridLayout *effectWidgetLayoutAudio;
	
	ClipViewItem *currentClip;
	
signals:
	void updateFrame();
};
#endif // FXSETTINGSPAGE_H