#include "filterwidget.h"

#include "headereffect.h"

#include "sliderdouble.h"
#include "sliderint.h"
#include "colorchooser.h"



FilterWidget::FilterWidget( QWidget *parent, Clip *c, Filter *f ) : QWidget( parent )
{
	clip = c;
	filter = f;
	
	QList<Parameter*> parameters = filter->getParameters();
	
	QBoxLayout* box = new QBoxLayout( QBoxLayout::TopToBottom, this );
	box->setContentsMargins( 0, 0, 0, 0 );
	
	HeaderEffect *header = new HeaderEffect( this, filter->getFilterName() );
	box->addWidget( header );
	connect( header, SIGNAL(deleteFilter()), this, SLOT(deleteFilter()) );
	
	int i;
	for ( i = 0; i < parameters.count(); ++i ) {
		Parameter *p = parameters[i];
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
			case Parameter::PCOLOR: {
				pw = new ColorChooser( this, p, clip != 0 );
				break;
			}
		}
		if ( pw ) {
			paramWidgets.append( pw );
			connect( pw, SIGNAL(valueChanged(Parameter*,QVariant)), this, SLOT(valueChanged(Parameter*,QVariant)) );
			connect( pw, SIGNAL(showAnimation(ParameterWidget*,Parameter*)), this, SLOT(showAnimation(ParameterWidget*,Parameter*)) );
			box->addLayout( pw->getLayout() );
		}
	}
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
