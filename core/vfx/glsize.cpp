#include <QBoxLayout>

#include "vfx/glsize.h"



GLSize::GLSize( QString id, QString name ) : GLFilter( id, name )
{
	resize = new GLResize( "GLResize" );
	padding = new GLPadding( "GLPadding" );
	resizeActive = true;
}



GLSize::~GLSize()
{
	delete padding;
	delete resize;
}



QString GLSize::getDescriptor( Frame *src, Profile *p )
{
	QString s;
	QList<Parameter*> params = resize->getParameters();

	if ( !params[0]->graph.keys.count() && 100.0 == getParamValue( params[0], src->pts() ).toDouble() ) {
		resizeActive = false;
	}
	else {
		s = resize->getDescriptor( src, p );
		resizeActive = true;
	}

	return s + padding->getDescriptor( src, p );
}



bool GLSize::process( const QList<Effect*> &el, Frame *src, Profile *p )
{
	if ( el.count() == 2 ) {
		QList<Effect*> l1;
		l1.append( el.at(0) );
		QList<Effect*> l2;
		l2.append( el.at(1) );
		return resize->process( l1, src, p )
			&& padding->process( l2, src, p );
	}
	
	return padding->process( el, src, p );
}



QList<Effect*> GLSize::getMovitEffects()
{
	QList<Effect*> list;
	if ( resizeActive )
		list.append( resize->getMovitEffects() );
	list.append( padding->getMovitEffects() );
	return list;
}



void GLSize::setPosition( double p )
{
	Filter::setPosition( p );
	resize->setPosition( p );
	padding->setPosition( p );
}



void GLSize::setLength( double len )
{
	Filter::setLength( len );
	resize->setLength( len );
	padding->setLength( len );
}



QList<Parameter*> GLSize::getParameters()
{
	return resize->getParameters() + padding->getParameters();
}
