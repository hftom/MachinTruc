#ifndef GLPUSH_H
#define GLPUSH_H

#include "vfx/glfilter.h"



static const char *MyPushEffect_shader=
"vec4 PREFIX(in1)( vec2 tc ) {\n"
"	vec4 sum = vec4(0.0);\n"
"	for ( float i = 0.0; i < PREFIX(loop); ++i ) {\n"
"		vec2 coord = tc + ( i * PREFIX(texSize) );\n"
"#if VERTICAL\n"
"#if DIRECTION\n"
"		if ( coord.y < 0.0 )\n"
"			sum += INPUT2( vec2( coord.x, coord.y + 1.0 ) );\n"
"#else\n"
"		if ( coord.y > 1.0 )\n"
"			sum += INPUT2( vec2( coord.x, coord.y - 1.0 ) );\n"
"#endif\n"
"#else\n"
"#if DIRECTION\n"
"		if ( coord.x > 1.0 )\n"
"			sum += INPUT2( vec2( coord.x - 1.0, coord.y ) );\n"
"#else\n"
"		if ( coord.x < 0.0 )\n"
"			sum += INPUT2( vec2( coord.x + 1.0, coord.y ) );\n"
"#endif\n"
"#endif\n"
"		else\n"
"			sum += INPUT1( coord );\n"
"	}\n"
"	return sum / PREFIX(loop);\n"
"}\n"
"\n"
"vec4 PREFIX(in2)( vec2 tc ) {\n"
"	vec4 sum = vec4(0.0);\n"
"	for ( float i = 0.0; i < PREFIX(loop); ++i ) {\n"
"		vec2 coord = tc + ( i * PREFIX(texSize) );\n"
"#if VERTICAL\n"
"#if DIRECTION\n"
"		if ( coord.y > 1.0 )\n"
"			sum += INPUT1( vec2( coord.x, coord.y - 1.0 ) );\n"
"#else\n"
"		if ( coord.y < 0.0 )\n"
"			sum += INPUT1( vec2( coord.x, coord.y + 1.0 ) );\n"
"#endif\n"
"#else\n"
"#if DIRECTION\n"
"		if ( coord.x < 0.0 )\n"
"			sum += INPUT1( vec2( coord.x + 1.0, coord.y ) );\n"
"#else\n"
"		if ( coord.x > 1.0 )\n"
"			sum += INPUT1( vec2( coord.x - 1.0, coord.y ) );\n"
"#endif\n"
"#endif\n"
"		else\n"
"			sum += INPUT2( coord );\n"
"	}\n"
"	return sum / PREFIX(loop);\n"
"}\n"
"\n"
"vec4 FUNCNAME( vec2 tc ) {\n"
"#if VERTICAL\n"
"#if DIRECTION\n"
"	if ( tc.y >= PREFIX(position) )\n"
"		return PREFIX(in1)( tc - vec2( 0.0, PREFIX(position) ) );\n"
"	return PREFIX(in2)( tc + vec2( 0.0, 1.0 - PREFIX(position) ) );\n"
"#else\n"
"	if ( tc.y >= 1.0 - PREFIX(position) )\n"
"		return PREFIX(in2)( tc - vec2( 0.0, 1.0 - PREFIX(position) ) );\n"
"	return PREFIX(in1)( tc + vec2( 0.0, PREFIX(position) ) );\n"
"#endif\n"
"#else\n"
"#if DIRECTION\n"
"	if ( tc.x >= 1.0 - PREFIX(position) )\n"
"		return PREFIX(in2)( tc - vec2( 1.0 - PREFIX(position), 0.0 ) );\n"
"	return PREFIX(in1)( tc + vec2( PREFIX(position), 0.0 ) );\n"
"#else\n"
"	if ( tc.x >= PREFIX(position) )\n"
"		return PREFIX(in1)( tc - vec2( PREFIX(position), 0.0 ) );\n"
"	return PREFIX(in2)( tc + vec2( 1.0 - PREFIX(position), 0.0 ) );\n"
"#endif\n"
"#endif\n"
"#undef VERTICAL\n"
"#undef DIRECTION\n"
"}\n";



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
		QString s = MyPushEffect_shader;
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
