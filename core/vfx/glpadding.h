#ifndef GLPADDING_H
#define GLPADDING_H

#include "vfx/glfilter.h"



class GLPadding : public GLFilter
{
public:
	GLPadding( QString id = "PaddingAuto", QString name = "PaddingAuto" );
	~GLPadding();

	QString getDescriptor( Frame *src, Profile *p );
	bool process( const QList<Effect*> &el, Frame *src, Profile *p );

	QList<Effect*> getMovitEffects();
	
private:
	void preProcess( Frame *src, Profile *p );

	double left, top;
};

#endif //GLPADDING_H
