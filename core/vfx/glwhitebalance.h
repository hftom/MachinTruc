#ifndef GLWHITEBALANCE_H
#define GLWHITEBALANCE_H

#include "vfx/glfilter.h"



class GLWhiteBalance : public GLFilter
{
public:
	GLWhiteBalance( QString id, QString name );
	~GLWhiteBalance();

	bool process( const QList<Effect*> &el, Frame *src, Profile *p );

	QList<Effect*> getMovitEffects();

private:
	Parameter *temperature;
	Parameter *neutralColor;
};

#endif //GLWHITEBALANCE_H