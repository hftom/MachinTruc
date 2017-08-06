#ifndef ANIMEDITOR_H
#define ANIMEDITOR_H

#include <QWidget>

#include "ui_animeditor.h"
#include "animation/animscene.h"



class AnimEditor : public QWidget, private Ui::EditorAnim
{
	Q_OBJECT
public:
	AnimEditor( QWidget *parent );

	void setCurrentParam( FilterWidget *f, ParameterWidget *pw, Parameter *p );

public slots:
	void filterDeleted( Clip *c, QSharedPointer<Filter> f );
	void ovdUpdate( QList<OVDUpdateMessage> msg );
	void setCursorPos( double pts, bool isPlaying );

private slots:
	void removeGraph();
	void editorQuit();

private:
	AnimScene *animScene;
	AnimItem *animItem;

signals:
	void ovdValueChanged(ParameterWidget*);
	void quitEditor();
	void updateFrame();
};

#endif // ANIMEDITOR_H
