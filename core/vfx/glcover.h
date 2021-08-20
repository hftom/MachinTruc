#ifndef GLCOVER_H
#define GLCOVER_H

#include "vfx/glfilter.h"



class MyCoverEffect : public Effect {
public:
	MyCoverEffect() : vertical(0), direction(0), uncover(0), position(0), loop(1), texSize(0,0) {
		register_int( "vertical", &vertical );
		register_int( "direction", &direction );
		register_int( "uncover", &uncover );
		register_float( "position", &position );
		register_float( "loop", &loop );
		register_vec2( "texSize", (float*)&texSize );
	}
	
	virtual std::string effect_type_id() const { return "MyCoverEffect"; }
	std::string output_fragment_shader() { 
		QString s = GLFilter::getQStringShader("cover.frag");
		if ( vertical )
			s.prepend( "#define VERTICAL 1\n" );
		else
			s.prepend( "#define VERTICAL 0\n" );
		if ( direction )
			s.prepend( "#define DIRECTION 1\n" );
		else
			s.prepend( "#define DIRECTION 0\n" );
		if ( uncover )
			s.prepend( "#define SOURCE1 INPUT2\n#define SOURCE2 INPUT1\n" );
		else
			s.prepend( "#define SOURCE1 INPUT1\n#define SOURCE2 INPUT2\n" );
		return s.toLatin1().data();
	}

	virtual bool needs_texture_bounce() const { return loop > 1; }
	virtual bool needs_srgb_primaries() const { return false; }
	virtual unsigned num_inputs() const { return 2; }

private:
	int vertical, direction, uncover;
	float position, loop;
	Point2D texSize;
};



class GLCover : public GLFilter
{
	Q_OBJECT
public:
	GLCover( QString id, QString name );

	QString getDescriptor( double pts, Frame *src, Profile *p );
	bool process( const QList<Effect*>&, double pts, Frame *first, Frame *second, Profile *p );
	QList<Effect*> getMovitEffects();

protected:
	Parameter *position, *vertical, *direction, *uncover, *motionBlur;
};

#endif //GLCOVER_H
