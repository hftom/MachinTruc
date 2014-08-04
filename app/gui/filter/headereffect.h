#ifndef HEADEREFFECT_H
#define HEADEREFFECT_H

#include <QWidget>

#include "ui_effectheader.h"



class HeaderEffect : public QWidget, private Ui::EffectHeader
{
	Q_OBJECT
public:
	HeaderEffect( QWidget *parent, QString name );
	
protected:
	void mousePressEvent( QMouseEvent *event );
	void mouseMoveEvent( QMouseEvent *event );
	void mouseReleaseEvent( QMouseEvent *event );
	
private slots:
	void buttonClicked();
	
signals:
	void deleteFilter();
};

#endif // HEADEREFFECT_H
