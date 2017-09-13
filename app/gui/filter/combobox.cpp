#include <QLabel>

#include "combobox.h"



ComboBox::ComboBox( QWidget *parent, Parameter *p ) : ParameterWidget( parent, p )
{
	box = new QBoxLayout( QBoxLayout::LeftToRight );
	box->setContentsMargins( 0, 0, 0, 0 );
	combo = new QComboBox();
	combo->addItem(tr("None"));
	widgets.append( combo );
	box->addWidget(new QLabel(p->name));
	box->addWidget( combo );

	connect( combo, SIGNAL(currentIndexChanged(int)), this, SLOT(stateChanged(int)) );
}



void ComboBox::addItem(QString item, QWidget *widget)
{
	combo->addItem(item);
	itemList.append(widget);
}



void ComboBox::initialize()
{
	combo->setCurrentIndex(param->value.toInt());
	showItem();
}



void ComboBox::showItem()
{
	int val = combo->currentIndex();
	for (int i = 0; i < itemList.count(); ++i) {
		itemList.at(i)->hide();
	}
	int v = val - 1;
	if (v >= 0 && v < itemList.count()) {
		itemList.at(v)->show();
	}
}



void ComboBox::stateChanged( int val )
{
	showItem();

	emit valueChanged( param, QVariant(val) );
}