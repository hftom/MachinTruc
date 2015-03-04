#ifndef GLCOVER_H
#define GLCOVER_H

#include "vfx/glfilter.h"



static const char *MyCoverEffect_shader=
"vec4 FUNCNAME( vec2 tc ) {\n"
"#if VERTICAL\n"
"#if REVERSED\n"
"	if ( tc.y >= PREFIX(position) )\n"
"		return INPUT1( tc );\n"
"	return INPUT2( tc + vec2( 0.0, 1.0 - PREFIX(position) ) );\n"
"#else\n"
"	if ( tc.y >= 1.0 - PREFIX(position) )\n"
"		return INPUT2( tc - vec2( 0.0, 1.0 - PREFIX(position) ) );\n"
"	return INPUT1( tc );\n"
"#endif\n"
"#else\n"
"#if REVERSED\n"
"	if ( tc.x >= 1.0 - PREFIX(position) )\n"
"		return INPUT2( tc - vec2( 1.0 - PREFIX(position), 0.0 ) );\n"
"	return INPUT1( tc );\n"
"#else\n"
"	if ( tc.x >= PREFIX(position) )\n"
"		return INPUT1( tc );\n"
"	return INPUT2( tc + vec2( 1.0 - PREFIX(position), 0.0 ) );\n"
"#endif\n"
"#endif\n"
"#undef VERTICAL\n"
"#undef REVERSED\n"
"}\n";



class MyCoverEffect : public Effect {
public:
	MyCoverEffect() : vertical(0), reversed(0), position(0) {
		register_int( "vertical", &vertical );
		register_int( "reversed", &reversed );
		register_float( "position", &position );
	}
	
	virtual std::string effect_type_id() const { return "MyCoverEffect"; }
	std::string output_fragment_shader() { 
		QString s = MyCoverEffect_shader;
		if ( vertical )
			s.prepend( "#define VERTICAL 1\n" );
		else
			s.prepend( "#define VERTICAL 0\n" );
		if ( reversed )
			s.prepend( "#define REVERSED 1\n" );
		else
			s.prepend( "#define REVERSED 0\n" );
		return s.toLatin1().data();
	}

	virtual bool needs_srgb_primaries() const { return false; }
	virtual unsigned num_inputs() const { return 2; }

private:
	int vertical, reversed;
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
	Parameter *position, *vertical, *reversed;
};

class GLCoverRL : public GLCover
{
public:
	GLCoverRL( QString id, QString name ) : GLCover( id, name ) {
		reversed->value = 1;
	}
};

class GLCoverTB : public GLCover
{
public:
	GLCoverTB( QString id, QString name ) : GLCover( id, name ) {
		vertical->value = 1;
	}
};

class GLCoverBT : public GLCover
{
public:
	GLCoverBT( QString id, QString name ) : GLCover( id, name ) {
		vertical->value = 1;
		reversed->value = 1;
	}
};

#endif //GLCOVER_H
