#ifndef HEADEREFFECT_H
#define HEADEREFFECT_H

#include <QWidget>

#include "ui_effectheader.h"
#include "engine/clip.h"



class HeaderEffect : public QWidget, private Ui::EffectHeader
{
	Q_OBJECT
public:
	HeaderEffect( Clip *c, Filter *f );
	
protected:
	void mousePressEvent( QMouseEvent *event );
	void mouseMoveEvent( QMouseEvent *event );
	void mouseReleaseEvent( QMouseEvent *event );
	
private:
	Clip *clip;
	Filter *filter;
	
private slots:
	void buttonClicked();
	
signals:
	void filterDeleted( Clip*, Filter* );

};
#endif // HEADEREFFECT_H