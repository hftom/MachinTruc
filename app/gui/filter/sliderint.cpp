#include "sliderint.h"



SliderInt::SliderInt( QWidget *parent, Parameter *p, bool keyframeable ) : ParameterWidget( parent, p )
{
	QBoxLayout *b1 = new QBoxLayout( QBoxLayout::LeftToRight );
	b1->setContentsMargins( 0, 0, 0, 0 );
	widgets.append( label = new QLabel( p->name ) );
	b1->addWidget( label );
	
	QBoxLayout *b2 = new QBoxLayout( QBoxLayout::LeftToRight );
	b2->setContentsMargins( 0, 0, 0, 0 );
	widgets.append( fs = new FSlider( NULL ) );
	widgets.append( spin = new QSpinBox() );
	fs->setRange( p->min.toInt(), p->max.toInt() );
	spin->setRange( p->min.toInt(), p->max.toInt() );
	spin->setSuffix( p->suffix );
	b2->addWidget( fs );
	b2->addWidget( spin );
	
	if ( keyframeable && p->keyframeable ) {
		addAnimBtn( NULL );
		b1->addWidget( animBtn );
	}
	
	box = new QBoxLayout( QBoxLayout::TopToBottom );
	box->setContentsMargins( 0, 0, 0, 0 );
	box->addLayout( b1 );
	box->addLayout( b2 );

	connect( fs, SIGNAL(valueChanged(int)), this, SLOT(sliderChanged(int)) );
	connect( spin, SIGNAL(valueChanged(int)), this, SLOT(spinChanged(int)) );
	
	fs->setValue( p->value.toInt() );
}



void SliderInt::spinChanged( int val )
{
	fs->blockSignals( true );
	fs->setValue( val );
	fs->blockSignals( false );
	
	if ( !animActive )
		emit valueChanged( param, QVariant(val) );
	else
		emit keyValueChanged( param, QVariant(val) );
}



void SliderInt::sliderChanged( int val )
{
	spin->blockSignals( true );
	spin->setValue( val );
	spin->blockSignals( false );
	
	if ( !animActive )
		emit valueChanged( param, QVariant(val) );
	else
		emit keyValueChanged( param, QVariant(val) );
}



void SliderInt::animValueChanged( double val )
{
	fs->blockSignals( true );
	spin->blockSignals( true );
	spin->setValue( val );
	fs->setValue( val );
	fs->blockSignals( false );
	spin->blockSignals( false );
}
