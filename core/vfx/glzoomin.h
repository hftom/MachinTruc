#ifndef GLZOOMIN_H
#define GLZOOMIN_H

#include "vfx/glsize.h"



class GLZoomIn : public GLSize
{
public:
	GLZoomIn( QString id, QString name );
	~GLZoomIn();
	
	bool process( const QList<Effect*>&, double pts, Frame *first, Frame *second, Profile *p );
	QList<Effect*> getMovitEffects();
	QList<Effect*> getMovitEffectsFirst();
	QList<Effect*> getMovitEffectsSecond();
	virtual QString getDescriptor( double pts, Frame *src, Profile *p );
	QString getDescriptorFirst( double pts, Frame *f, Profile *p );
	QString getDescriptorSecond( double pts, Frame *f, Profile *p );
	
private:
	QList<Effect*> firstList, secondList;
	Parameter *rotateStart, *inverse;

};

#endif //GLZOOMIN_H
