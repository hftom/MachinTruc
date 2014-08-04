#include "sliderdouble.h"



SliderDouble::SliderDouble( QWidget *parent, Parameter *p, bool keyframeable ) : ParameterWidget( parent, p )
{
	QBoxLayout *b1 = new QBoxLayout( QBoxLayout::LeftToRight );
	b1->setContentsMargins( 0, 0, 0, 0 );
	widgets.append( label = new QLabel( p->name, parent ) );
	b1->addWidget( label );
	
	QBoxLayout *b2 = new QBoxLayout( QBoxLayout::LeftToRight );
	b2->setContentsMargins( 0, 0, 0, 0 );
	widgets.append( fs = new FSlider( parent ) );
	widgets.append( spin = new QDoubleSpinBox( parent ) );
	fs->setRange( p->min.toDouble() * 100, p->max.toDouble() * 100 );
	spin->setRange( p->min.toDouble(), p->max.toDouble() );
	spin->setSuffix( p->suffix );
	spin->setSingleStep( 0.01 );
	b2->addWidget( fs );
	b2->addWidget( spin );
	
	if ( keyframeable && p->keyframeable ) {
		addAnimBtn( parent );
		b1->addWidget( animBtn );
	}
	
	box = new QBoxLayout( QBoxLayout::TopToBottom );
	box->setContentsMargins( 0, 0, 0, 0 );
	box->addLayout( b1 );
	box->addLayout( b2 );

	connect( fs, SIGNAL(valueChanged(int)), this, SLOT(sliderChanged(int)) );
	connect( spin, SIGNAL(valueChanged(double)), this, SLOT(spinChanged(double)) );
	
	fs->setValue( p->value.toDouble() * 100.0 );
}



void SliderDouble::spinChanged( double val )
{
	fs->blockSignals( true );
	fs->setValue( val * 100 );
	fs->blockSignals( false );
	
	if ( !animActive )
		emit valueChanged( param, QVariant(val * 100) );
	else
		emit keyValueChanged( param, QVariant(val * 100) );
}



void SliderDouble::sliderChanged( int val )
{
	spin->blockSignals( true );
	spin->setValue( val / 100.0 );
	spin->blockSignals( false );
	
	if ( !animActive )
		emit valueChanged( param, QVariant(val) );
	else
		emit keyValueChanged( param, QVariant(val) );
}



void SliderDouble::animValueChanged( double val )
{
	fs->blockSignals( true );
	spin->blockSignals( true );
	fs->setValue( val * 100 );
	spin->setValue( val );
	fs->blockSignals( false );
	spin->blockSignals( false );
}
