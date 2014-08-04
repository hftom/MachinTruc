#ifndef GLLIFTGAMMAGAIN_H
#define GLLIFTGAMMAGAIN_H

#include "vfx/glfilter.h"



class GLLiftGammaGain : public GLFilter
{
public:
    GLLiftGammaGain( QString id, QString name );
    ~GLLiftGammaGain();

    bool process( const QList<Effect*> &el, Frame *src, Profile *p );

	QList<Effect*> getMovitEffects();

private:
    Parameter *liftR, *liftG, *liftB;
	Parameter *gammaR, *gammaG, *gammaB;
	Parameter *gainR, *gainB, *gainG;
};

#endif //GLLIFTGAMMAGAIN_H
