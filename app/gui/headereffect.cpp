#include "headereffect.h"



HeaderEffect::HeaderEffect( Clip *c, Filter *f )
{
	clip = c;
	filter = f;
	
	setupUi( this );
	
	QColor bg = palette().color( QPalette::Normal, QPalette::Highlight );
	QColor fg = palette().color( QPalette::Normal, QPalette::HighlightedText );
	setStyleSheet( QString( "background-color: rgb(%1,%2,%3); color: rgb(%4,%5,%6)" ).arg( bg.red() ).arg( bg.green() ).arg( bg.blue() ).arg( fg.red() ).arg( fg.green() ).arg( fg.blue() ) );
	QColor button = toolButton->palette().color( QPalette::Normal, QPalette::Button );
	toolButton->setStyleSheet( QString( "background-color: rgb(%1,%2,%3)" ).arg( button.red() ).arg( button.green() ).arg( button.blue() ) );
	
	label->setText( filter->getFilterName() );
	
	connect( toolButton, SIGNAL(clicked()), this, SLOT(buttonClicked()) );
}



void HeaderEffect::buttonClicked()
{
	emit filterDeleted( clip, filter );
}



void HeaderEffect::mousePressEvent( QMouseEvent *event )
{
	qDebug() << "HeaderEffect::mousePressEvent";
}



void HeaderEffect::mouseMoveEvent( QMouseEvent *event )
{
	qDebug() << "HeaderEffect::mouseMoveEvent";
}



void HeaderEffect::mouseReleaseEvent( QMouseEvent *event )
{
	qDebug() << "HeaderEffect::mouseReleaseEvent";
}
