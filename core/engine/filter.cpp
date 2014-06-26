#include <QDebug>
#include <QBoxLayout>
#include <QLabel>

#include "filter.h"



Filter::Filter( QString id, QString name )
{
	identifier = id;
	filterName = name;
	refCount = 0;
	use();
}



Filter::~Filter()
{
	while ( !parameters.isEmpty() )
		delete parameters.takeFirst();
}



void Filter::use()
{
	rcMutex.lock();
	++refCount;
	rcMutex.unlock();
}



void Filter::release()
{
	rcMutex.lock();
	--refCount;
	//qDebug() << "Filter::release" << refCount << filterName << this;
	rcMutex.unlock();
	
	if ( !refCount ) {
		//qDebug() << filterName << "delete";
		delete this;
	}
}



void Filter::addParameter( QString name, int type, QVariant min, QVariant max, bool keyframeable, void *value )
{
	Parameter *param = new Parameter();
	param->name = name;
	param->type = type;
	param->min = min;
	param->max = max;
	param->keyframeable = keyframeable;
	param->value = value;
	parameters.append( param );
}



QWidget* Filter::getWidget()
{
	if ( !parameters.count() )
		return NULL;

	QWidget *widg = new QWidget();
	widg->setMinimumWidth( 150 );
	QBoxLayout* box = new QBoxLayout( QBoxLayout::TopToBottom, widg );
	
	int i;
	for ( i = 0; i < parameters.count(); ++i ) {
		Parameter *p = parameters.at ( i );
		QLayout *layout = NULL;
		switch ( p->type ) {
			case PFLOAT:
				layout = layoutPFLOAT( widg, p );
				break;
			case PINT:
				layout = layoutPINT( widg, p );
				break;
		}
		if ( layout ) {
			box->addLayout( layout );
		}
	}
	
	return widg;
}



void Filter::valueChanged( Parameter *p, QVariant val )
{
	switch ( p->type ) {
		case PFLOAT:
			*(float*)(p->value) = val.toInt() / 100.0;
			break;
		case PINT:
			*(int*)(p->value) = val.toInt();
			break;
	}
	emit updateFrame();
}



QLayout* Filter::layoutPFLOAT( QWidget *w, Parameter *p )
{
	QBoxLayout *box = new QBoxLayout( QBoxLayout::TopToBottom );
	box->addWidget( new QLabel( p->name, w ) );
	FSlider *sl = new FSlider( p, w );
	sl->setRange( p->min.toFloat() * 100, p->max.toFloat() * 100 );
	sl->setValue( *(float*)(p->value) * 100.0 );
	connect( sl, SIGNAL(valueChanged(Parameter*,QVariant)), this, SLOT(valueChanged(Parameter*,QVariant)) );
	box->addWidget( sl );
	
	return box;
}



QLayout* Filter::layoutPINT( QWidget *w, Parameter *p )
{
	QBoxLayout *box = new QBoxLayout( QBoxLayout::TopToBottom );
	box->addWidget( new QLabel( p->name, w ) );
	FSlider *sl = new FSlider( p, w );
	sl->setRange( p->min.toInt(), p->max.toInt() );
	sl->setValue( *(int*)(p->value) );
	connect( sl, SIGNAL(valueChanged(Parameter*,QVariant)), this, SLOT(valueChanged(Parameter*,QVariant)) );
	box->addWidget( sl );
	
	return box;
}