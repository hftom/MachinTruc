#ifndef GLSIZE_H
#define GLSIZE_H

#include "glfilter.h"



class GLSize : public GLFilter
{
public:
	GLSize( QString id, QString name );
	~GLSize();
 
	QString getDescriptor( Frame *src, Profile *p );
	bool process( const QList<Effect*> &el, Frame *src, Profile *p );

	QList<Effect*> getMovitEffects();

		
private:
	void preProcessResize( Frame *src, Profile *p );
	void preProcessPadding( Frame *src, Profile *p );

	Parameter *sizePercent, *xOffsetPercent, *yOffsetPercent;
	double left, top;
	bool resizeActive;
};

#endif //GLSIZE_H
