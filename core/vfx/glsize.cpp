#include <QBoxLayout>

#include "vfx/glsize.h"



GLSize::GLSize( QString id, QString name ) : GLFilter( id, name )
{
	resize = new GLResize();
	padding = new GLPadding();
	
	connect( resize, SIGNAL(updateFrame()), this, SIGNAL(updateFrame()) );
	connect( padding, SIGNAL(updateFrame()), this, SIGNAL(updateFrame()) );
}



GLSize::~GLSize()
{
	delete padding;
	delete resize;
}



bool GLSize::preProcess( Frame *src, Profile *p )
{
	return resize->preProcess( src, p ) && padding->preProcess( src, p );
}



bool GLSize::process( const QList<Effect*> &el, Frame *src, Profile *p )
{
	QList<Effect*> l1;
	l1.append( el.at(0) );
	QList<Effect*> l2;
	l2.append( el.at(1) );
	
	return resize->process( l1, src, p )
		&& padding->process( l2, src, p );
}



QList<Effect*> GLSize::getMovitEffects()
{
	QList<Effect*> list;
	list.append( resize->getMovitEffects() );
	list.append( padding->getMovitEffects() );
	return list;
}



QWidget* GLSize::getWidget()
{
	QWidget *widg = new QWidget();
	widg->setMinimumWidth( 150 );
	QBoxLayout* box = new QBoxLayout( QBoxLayout::TopToBottom, widg );
	box->addWidget( resize->getWidget() );
	box->addWidget( padding->getWidget() );
	
	return widg;
}
