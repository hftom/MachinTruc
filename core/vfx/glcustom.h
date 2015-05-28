#ifndef GLCUSTOM_H
#define GLCUSTOM_H

#include <movit/effect_util.h>

#include "vfx/glfilter.h"



class DynamicParam
{
public:
	DynamicParam() {
		f[0] = f[1] = f[2] = f[3] = 0.0f;
	}

	float f[4];
};
	



class MCustomEffect : public Effect
{
public:
	MCustomEffect( QString shader ) : time( 0 ), texelSize( 1, 1 ) {
		register_float( "time", &time );
		register_vec2( "texelSize", (float*)&texelSize );
		
		int faulty;
		QList<Parameter> list = Parameter::parseShaderParams( shader, faulty );
		for ( int i = 0; i < list.count(); ++i ) {
			Parameter p = list.at(i);
			if ( p.type == Parameter::PDOUBLE ) {
				dynParams.append( DynamicParam() );
				register_float( p.id.toLatin1().data(), &dynParams.last().f[0] );
			}
			else if ( p.type == Parameter::PRGBCOLOR ) {
				dynParams.append( DynamicParam() );
				register_vec3( p.id.toLatin1().data(), &dynParams.last().f[0] );
			}
			else if ( p.type == Parameter::PRGBACOLOR ) {
				dynParams.append( DynamicParam() );
				register_vec4( p.id.toLatin1().data(), &dynParams.last().f[0] );
			}
		}
		shaderText = shader;
	}
	std::string effect_type_id() const { return "MCustomEffect"; }
	std::string output_fragment_shader() { return shaderText.toLatin1().data(); }
	
private:
	float time;
	Point2D texelSize;
	QList<DynamicParam> dynParams;
	QString shaderText;
};



class GLCustom : public GLFilter
{
public:
	GLCustom( QString id, QString name );
	~GLCustom();

	virtual bool process( const QList<Effect*> &el, Frame *src, Profile *p );
	virtual QString getDescriptor( Frame *src, Profile *p );
	virtual QString getFilterName();

	virtual QList<Effect*> getMovitEffects();
	
	void setCustomParams( QString shader );
	
private:
	Parameter *editor;
	QList<Parameter*> shaderParams;
	QString currentShader;
	unsigned char version;
	QString filterName;
	
	QMutex mutex;
};

#endif // GLCUSTOM_H
