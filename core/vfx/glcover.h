#ifndef GLCOVER_H
#define GLCOVER_H

#include "vfx/glfilter.h"



static const char *MyCoverEffect_shader=
"vec4 FUNCNAME( vec2 tc ) {\n"
"#if VERTICAL\n"
"#if DIRECTION\n"
"	if ( tc.y >= PREFIX(position) )\n"
"		return SOURCE1( tc );\n"
"	return SOURCE2( tc + vec2( 0.0, 1.0 - PREFIX(position) ) );\n"
"#else\n"
"	if ( tc.y >= 1.0 - PREFIX(position) )\n"
"		return SOURCE2( tc - vec2( 0.0, 1.0 - PREFIX(position) ) );\n"
"	return SOURCE1( tc );\n"
"#endif\n"
"#else\n"
"#if DIRECTION\n"
"	if ( tc.x >= 1.0 - PREFIX(position) )\n"
"		return SOURCE2( tc - vec2( 1.0 - PREFIX(position), 0.0 ) );\n"
"	return SOURCE1( tc );\n"
"#else\n"
"	if ( tc.x >= PREFIX(position) )\n"
"		return SOURCE1( tc );\n"
"	return SOURCE2( tc + vec2( 1.0 - PREFIX(position), 0.0 ) );\n"
"#endif\n"
"#endif\n"
"#undef VERTICAL\n"
"#undef DIRECTION\n"
"#undef SOURCE1\n"
"#undef SOURCE2\n"
"}\n";



class MyCoverEffect : public Effect {
public:
	MyCoverEffect() : vertical(0), direction(0), uncover(0), position(0) {
		register_int( "vertical", &vertical );
		register_int( "direction", &direction );
		register_int( "uncover", &uncover );
		register_float( "position", &position );
	}
	
	virtual std::string effect_type_id() const { return "MyCoverEffect"; }
	std::string output_fragment_shader() { 
		QString s = MyCoverEffect_shader;
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

	virtual bool needs_srgb_primaries() const { return false; }
	virtual unsigned num_inputs() const { return 2; }

private:
	int vertical, direction, uncover;
	float position;
};



class GLCover : public GLFilter
{
public:
	GLCover( QString id, QString name );

	QString getDescriptor( Frame *src, Profile *p );
	bool process( const QList<Effect*>&, Frame *src, Frame *dst, Profile *p );
	QList<Effect*> getMovitEffects();

protected:
	Parameter *position, *vertical, *direction, *uncover;
};

#endif //GLCOVER_H
