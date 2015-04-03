#include "filterwidget.h"

#include "sliderdouble.h"
#include "sliderint.h"
#include "colorchooser.h"
#include "checkbox.h"
#include "colorwheel.h"



FilterWidget::FilterWidget( QWidget *parent, Clip *c, QSharedPointer<Filter> f ) : QWidget( parent ),
	clip( c ),
	filter( f )
{	
	QList<Parameter*> parameters = filter->getParameters();
	
	QGridLayout* grid = new QGridLayout();
	grid->setContentsMargins( 0, 0, 0, 0 );
	int row = 0;
	
	int i;
	for ( i = 0; i < parameters.count(); ++i ) {
		Parameter *p = parameters[i];
		if ( p->hidden )
			continue;
		ParameterWidget *pw = NULL;
		switch ( p->type ) {
			case Parameter::PDOUBLE: {
				pw = new SliderDouble( this, p, clip != 0 );
				break;
			}
			case Parameter::PINT: {
				pw = new SliderInt( this, p, clip != 0 );
				break;
			}
			case Parameter::PBOOL: {
				pw = new CheckBox( this, p );
				break;
			}
			case Parameter::PRGBCOLOR: {
				pw = new ColorChooser( false, this, p, clip != 0 );
				break;
			}
			case Parameter::PRGBACOLOR: {
				pw = new ColorChooser( true, this, p, clip != 0 );
				break;
			}
			case Parameter::PCOLORWHEEL: {
				pw = new ColorWheel( this, p, clip != 0 );
				break;
			}
		}
		if ( pw ) {
			paramWidgets.append( pw );
			connect( pw, SIGNAL(valueChanged(Parameter*,QVariant)), this, SLOT(valueChanged(Parameter*,QVariant)) );
			connect( pw, SIGNAL(showAnimation(ParameterWidget*,Parameter*)), this, SLOT(showAnimation(ParameterWidget*,Parameter*)) );
			if ( p->layout.row != -1 )
				grid->addLayout( pw->getLayout(), p->layout.row, p->layout.column, p->layout.rowSpan, p->layout.columnSpan );
			else 
				grid->addLayout( pw->getLayout(), row++, 0 );
		}
		++row;
	}
	
	setLayout( grid );
}



void FilterWidget::setAnimActive( Parameter *p )
{
	int i;
	for ( i = 0; i < paramWidgets.count(); ++i ) {
		ParameterWidget *pw = paramWidgets[i];
		if ( pw->getParameter() == p ) {
			pw->setAnimActive( true );
		}
		else {
			pw->setAnimActive( false );
		}
	}
}



void FilterWidget::deleteFilter()
{
	if ( clip )
		emit filterDeleted( clip, filter );
	else
		emit filterSourceDeleted();
}



void FilterWidget::moveUp()
{
	if ( clip )
		emit filterMoveUp( clip, filter );
}



void FilterWidget::moveDown()
{
	if ( clip )
		emit filterMoveDown( clip, filter );
}



void FilterWidget::valueChanged( Parameter *p, QVariant val )
{
	switch ( p->type ) {
		case Parameter::PDOUBLE:
			p->value = val.toInt() / 100.0;
			break;
		default:
			p->value = val;
			break;
	}
	emit updateFrame();
}
