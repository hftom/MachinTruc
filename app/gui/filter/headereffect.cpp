#include <QDebug>

#include "headereffect.h"



HeaderEffect::HeaderEffect( QWidget *parent, QString name ) : QWidget( parent )
{
	setupUi( this );
	
	QColor bg = palette().color( QPalette::Normal, QPalette::Highlight );
	QColor fg = palette().color( QPalette::Normal, QPalette::HighlightedText );
	label->setStyleSheet( QString( "background-color: rgb(%1,%2,%3); color: rgb(%4,%5,%6)" ).arg( bg.red() ).arg( bg.green() ).arg( bg.blue() ).arg( fg.red() ).arg( fg.green() ).arg( fg.blue() ) );
	//QColor button = deleteBtn->palette().color( QPalette::Normal, QPalette::Button );
	//deleteBtn->setStyleSheet( QString( "background-color: rgb(%1,%2,%3)" ).arg( button.red() ).arg( button.green() ).arg( button.blue() ) );
	
	label->setText( name );
	
	connect( deleteBtn, SIGNAL(clicked()), this, SLOT(buttonClicked()) );
}



void HeaderEffect::buttonClicked()
{
	emit deleteFilter();
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
