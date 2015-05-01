#include "inputdouble.h"



InputDouble::InputDouble( QWidget *parent, Parameter *p, bool keyframeable ) : ParameterWidget( parent, p )
{
	box = new QBoxLayout( QBoxLayout::LeftToRight );
	box->setContentsMargins( 0, 0, 0, 0 );
	
	widgets.append( label = new QLabel( p->name ) );
	box->addWidget( label );
	widgets.append( spin = new QDoubleSpinBox() );
	spin->setRange( p->min.toDouble(), p->max.toDouble() );
	spin->setSuffix( p->suffix );
	spin->setSingleStep( 0.01 );
	box->addWidget( spin );
	box->addSpacerItem( new QSpacerItem( 1, 1, QSizePolicy::Expanding, QSizePolicy::Expanding ) );
	
	if ( keyframeable && p->keyframeable ) {
		addAnimBtn( NULL );
		box->addWidget( animBtn );
	}

	connect( spin, SIGNAL(valueChanged(double)), this, SLOT(spinChanged(double)) );
	
	spin->setValue( p->value.toDouble() );
}



void InputDouble::spinChanged( double val )
{
	if ( !animActive )
		emit valueChanged( param, QVariant(val * 100) );
	else
		emit keyValueChanged( param, QVariant(val * 100) );
}



void InputDouble::animValueChanged( double val )
{
	spin->blockSignals( true );
	spin->setValue( val );
	spin->blockSignals( false );
}



void InputDouble::ovdValueChanged()
{
	if ( !param->graph.keys.count() ) {
		spin->blockSignals( true );
		spin->setValue( param->value.toDouble() );
		spin->blockSignals( false );
	}
}
