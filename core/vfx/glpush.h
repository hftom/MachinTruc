#ifndef GLPUSH_H
#define GLPUSH_H

#include "vfx/glfilter.h"



class MyPushEffect : public Effect {
public:
	MyPushEffect() : vertical(0), direction(0), position(0), loop(1), texSize(0,0) {
		register_int( "vertical", &vertical );
		register_int( "direction", &direction );
		register_float( "position", &position );
		register_float( "loop", &loop );
		register_vec2( "texSize", (float*)&texSize );
	}
	
	virtual std::string effect_type_id() const { return "MyPushEffect"; }
	std::string output_fragment_shader() { 
		QString s = GLFilter::getQStringShader("push.frag");
		if ( vertical )
			s.prepend( "#define VERTICAL 1\n" );
		else
			s.prepend( "#define VERTICAL 0\n" );
		if ( direction )
			s.prepend( "#define DIRECTION 1\n" );
		else
			s.prepend( "#define DIRECTION 0\n" );
		return s.toLatin1().data();
	}

	virtual bool needs_texture_bounce() const { return loop > 1; }
	virtual bool needs_srgb_primaries() const { return false; }
	virtual unsigned num_inputs() const { return 2; }

private:
	int vertical, direction;
	float position, loop;
	Point2D texSize;
};



class GLPush : public GLFilter
{
	Q_OBJECT
public:
	GLPush( QString id, QString name );

	QString getDescriptor( double pts, Frame *src, Profile *p );
	bool process( const QList<Effect*>&, double pts, Frame *first, Frame *second, Profile *p );
	QList<Effect*> getMovitEffects();

protected:
	Parameter *position, *vertical, *direction, *motionBlur;
};

#endif //GLPUSH_H
