#ifndef GLHARDCUT_H
#define GLHARDCUT_H

#include "vfx/glfilter.h"



class MyHardCutEffect : public Effect {
public:
	MyHardCutEffect() : show_second( 0 ) {
		register_float( "show_second", &show_second );
	}	
	virtual std::string effect_type_id() const { return "MyHardCutEffect"; }
	std::string output_fragment_shader() { return GLFilter::getShader("hard_cut.frag"); }
	virtual bool needs_srgb_primaries() const { return false; }
	virtual unsigned num_inputs() const { return 2; }

private:
	float show_second;
};



class GLHardCut : public GLFilter
{
	Q_OBJECT
public:
	GLHardCut( QString id, QString name );

	bool process( const QList<Effect*>&, double pts, Frame *first, Frame *second, Profile *p );
	QList<Effect*> getMovitEffects();

protected:
	Parameter *position;
};

#endif //GLHARDCUT_H
