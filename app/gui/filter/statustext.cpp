#include "statustext.h"



StatusText::StatusText( QWidget *parent, Parameter *p ) : ParameterWidget( parent, p )
{
	box = new QBoxLayout( QBoxLayout::LeftToRight );
	box->setContentsMargins( 0, 0, 0, 0 );
	widgets.append( label = new QLabel( p->name ) );
	box->addWidget( label );
	widgets.append( text = new QLabel( p->value.toString() ) );
	box->addWidget( text );
	box->setStretch( 1, 1 );
}



void StatusText::statusUpdate( QString s )
{
	text->setText( s );
}
