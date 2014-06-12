#ifndef GLLIFTGAMMAGAIN_H
#define GLLIFTGAMMAGAIN_H

#include "vfx/glfilter.h"



class GLLiftGammaGain : public GLFilter
{
public:
    GLLiftGammaGain( QString id, QString name );
    ~GLLiftGammaGain();

    bool process( Effect *e, Frame *src, Profile *p );

	Effect* getMovitEffect();

private:
    float lift[3], gamma[3], gain[3];
};

#endif //GLLIFTGAMMAGAIN_H
