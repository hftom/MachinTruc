#ifndef GLLIFTGAMMAGAIN_H
#define GLLIFTGAMMAGAIN_H

#include "vfx/glmask.h"



class GLLiftGammaGain : public GLMask
{
	Q_OBJECT
public:
	GLLiftGammaGain( QString id, QString name );
	~GLLiftGammaGain();

	bool process( const QList<Effect*> &el, double pts, Frame *src, Profile *p );
	QString getDescriptor( double pts, Frame *src, Profile *p  );

	QList<Effect*> getMovitEffects();

private:
	Parameter *lift, *gamma, *gain;
};

#endif //GLLIFTGAMMAGAIN_H
