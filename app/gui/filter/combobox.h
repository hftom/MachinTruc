#ifndef COMBOBOX_H
#define COMBOBOX_H

#include <QComboBox>
#include <QBoxLayout>

#include "parameterwidget.h"



class ComboBox : public ParameterWidget
{
	Q_OBJECT
public:
	ComboBox( QWidget *parent, Parameter *p );
	QLayout *getLayout() { return box; }

	void addItem(QString item, QWidget *widget);
	void initialize();

	virtual void animValueChanged( double /*val*/ ) {};

private slots:
	void stateChanged( int val );

private:
	void showItem();

	QComboBox *combo;
	QBoxLayout *box;
	QList<QWidget*> itemList;
};

#endif // COMBOBOX_H
