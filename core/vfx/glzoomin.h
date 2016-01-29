#ifndef GLZOOMIN_H
#define GLZOOMIN_H

#include "vfx/glsize.h"



static const char *FirstOverlayEffect_frag =
"vec4 FUNCNAME(vec2 tc) {\n"
"	vec4 bottom = INPUT1(tc);\n"
"	vec4 top = INPUT2(tc);\n"
"	return bottom + (1.0 - bottom.a) * top;\n"
"}\n";



class FirstOverlayEffect : public Effect {
public:
	FirstOverlayEffect() {}
	virtual std::string effect_type_id() const { return "FirstOverlayEffect"; }
	std::string output_fragment_shader() { return FirstOverlayEffect_frag; }
	virtual bool needs_srgb_primaries() const { return false; }
	virtual unsigned num_inputs() const { return 2; }
	virtual bool one_to_one_sampling() const { return true; }
	virtual AlphaHandling alpha_handling() const { return INPUT_PREMULTIPLIED_ALPHA_KEEP_BLANK; }
};



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
