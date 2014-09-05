#ifndef GLSIZE_H
#define GLSIZE_H

#include "vfx/glpadding.h"
#include "vfx/glresize.h"



class GLSize : public GLFilter
{
public:
	GLSize( QString id, QString name );
	~GLSize();
 
	QString getDescriptor( Frame *src, Profile *p );
	bool process( const QList<Effect*> &el, Frame *src, Profile *p );

	QList<Effect*> getMovitEffects();
	
	// Filter virtuals
	void setPosition( double p );
	void setLength( double len );
	QList<Parameter*> getParameters();
		
private:
	GLPadding *padding;
	GLResize *resize;
	bool resizeActive;
};

#endif //GLSIZE_H
