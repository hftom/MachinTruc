#ifndef HEADEREFFECT_H
#define HEADEREFFECT_H

#include <QWidget>

#include "ui_effectheader.h"



class HeaderEffect : public QWidget, public Ui::EffectHeader
{
	Q_OBJECT
public:
	HeaderEffect( QString name, bool move = true );
	
signals:
	void deleteFilter();
	void moveUp();
	void moveDown();
};

#endif // HEADEREFFECT_H
