#include <QDebug>

#include "headereffect.h"



HeaderEffect::HeaderEffect( QString name, bool move )
{
	setupUi( this );
	
	QColor bg = palette().color( QPalette::Normal, QPalette::Highlight );
	QColor fg = palette().color( QPalette::Normal, QPalette::HighlightedText );
	label->setStyleSheet( QString( "background-color: rgb(%1,%2,%3); color: rgb(%4,%5,%6)" ).arg( bg.red() ).arg( bg.green() ).arg( bg.blue() ).arg( fg.red() ).arg( fg.green() ).arg( fg.blue() ) );
	//QColor button = deleteBtn->palette().color( QPalette::Normal, QPalette::Button );
	//deleteBtn->setStyleSheet( QString( "background-color: rgb(%1,%2,%3)" ).arg( button.red() ).arg( button.green() ).arg( button.blue() ) );
	
	if ( !move ) {
		upBtn->hide();
		downBtn->hide();
	}
	
	label->setText( name );
	
	connect( deleteBtn, SIGNAL(clicked()), this, SIGNAL(deleteFilter()) );
	connect( upBtn, SIGNAL(clicked()), this, SIGNAL(moveUp()) );
	connect( downBtn, SIGNAL(clicked()), this, SIGNAL(moveDown()) );
}
