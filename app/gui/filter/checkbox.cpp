#include "checkbox.h"



CheckBox::CheckBox( QWidget *parent, Parameter *p ) : ParameterWidget( parent, p )
{
	box = new QBoxLayout( QBoxLayout::TopToBottom );
	box->setContentsMargins( 0, 0, 0, 0 );
	check = new QCheckBox( p->name );
	widgets.append( check );
	box->addWidget( check );

	connect( check, SIGNAL(stateChanged(int)), this, SLOT(stateChanged(int)) );
	check->setCheckState( p->value.toInt() > 0 ? Qt::Checked : Qt::Unchecked );
}



void CheckBox::stateChanged( int val )
{
	int v = val == Qt::Checked ? 1 : 0;
	emit valueChanged( param, QVariant(v) );
}
