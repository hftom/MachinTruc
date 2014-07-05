#ifndef GLSIZE_H
#define GLSIZE_H

#include "vfx/glpadding.h"
#include "vfx/glresize.h"



class GLSize : public GLFilter
{
public:
    GLSize( QString id, QString name );
    ~GLSize();

	bool preProcess( Frame *src, Profile *p ); 
    bool process( const QList<Effect*> &el, Frame *src, Profile *p );

	QList<Effect*> getMovitEffects();
	QWidget* getWidget();
		
private:
	GLPadding *padding;
	GLResize *resize;
};

#endif //GLSIZE_H
