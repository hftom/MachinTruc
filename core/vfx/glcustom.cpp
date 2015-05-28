#include "engine/util.h"
#include "vfx/glcustom.h"



GLCustom::GLCustom( QString id, QString name ) : GLFilter( id, name ), version( 0 )
{
	QString shader = "//NAME:Horizontal symmetry\n";
	shader += "vec4 FUNCNAME( vec2 tc ) {\n";
	shader += "	if ( tc.x > 0.5 )\n";
	shader += "		tc.x = 1.0 - tc.x;\n";
	shader += "	return INPUT( tc );\n";
	shader += "}\n";
	editor = addParameter( "editor", tr("Editor:"), Parameter::PSHADEREDIT, shader, "", "", false );
	setCustomParams( shader );
}



GLCustom::~GLCustom()
{
}



QString GLCustom::getFilterName()
{
	if ( !filterName.isEmpty() )
		return filterName;
	return Filter::getFilterName();
}



void GLCustom::setCustomParams( QString shader )
{
	QMutexLocker ml( &mutex );
	removeParameters( &shaderParams );
	shaderParams.clear();
	int faulty;
	QList<Parameter> list = Parameter::parseShaderParams( shader, faulty );
	for ( int i = list.count() - 1; i >= 0; --i ) {
		Parameter p = list.at(i);
		if ( p.type == Parameter::PDOUBLE ) {
			Parameter *sp = addParameter( p.id, p.name, p.type, p.defValue, p.min, p.max, p.keyframeable );
			prependLastParameter();
			shaderParams.prepend( sp );
		}
		else if ( p.type == Parameter::PRGBCOLOR || p.type == Parameter::PRGBACOLOR ) {
			Parameter *sp = addParameter( p.id, p.name, p.type, p.defValue, p.defValue, p.defValue, false );
			prependLastParameter();
			shaderParams.prepend( sp );
		}
	}
	filterName = "";
	QStringList sl = shader.split( "\n" );
	for ( int i = 0; i < sl.count(); ++i ) {
		if ( sl[i].trimmed().startsWith( "//NAME:" ) ) {
			filterName = sl[i].trimmed().replace( 0, 7, "" );
			break;
		}
	}
}



QString GLCustom::getDescriptor( Frame *src, Profile *p )
{
	QMutexLocker ml( &mutex );
	QString shader = getParamValue( editor ).toString();
	if ( shader != currentShader ) {
		currentShader = shader;
		++version;
	}
	return QString( "%1 %2" ).arg( getIdentifier() ).arg( version );
}



bool GLCustom::process( const QList<Effect*> &el, Frame *src, Profile *p )
{
	QMutexLocker ml( &mutex );
	Q_UNUSED( p );

	bool ok = true;
	double pts = src->pts();
	Effect *e = el.at(0);
	for ( int i = 0; i < shaderParams.count(); ++i ) {
		Parameter *param = shaderParams.at( i );
		if ( param->type == Parameter::PDOUBLE )
			ok |= e->set_float( param->id.toLatin1().data(), getParamValue( param, pts ).toFloat() );
		else if ( param->type == Parameter::PRGBCOLOR ) {
			QColor c = getParamValue( param ).value<QColor>();
			// convert gamma and premultiply
			sRgbColorToLinear( c );
			RGBTriplet col = RGBTriplet( c.redF(), c.greenF(), c.blueF() );
			ok |= e->set_vec3( param->id.toLatin1().data(), (float*)&col );
		}
		else if ( param->type == Parameter::PRGBACOLOR ) {
			QColor c = getParamValue( param ).value<QColor>();
			// convert gamma and premultiply
			sRgbColorToPremultipliedLinear( c );
			RGBATuple col = RGBATuple( c.redF(), c.greenF(), c.blueF(), c.alphaF() );
			ok |= e->set_vec4( param->id.toLatin1().data(), (float*)&col );
		}
	}
		
	float ts[2] = { 1.0f / src->glWidth, 1.0f / src->glHeight };
	return ok && e->set_float( "time", src->pts() / MICROSECOND )
		&& e->set_vec2( "texelSize", ts );
}



QList<Effect*> GLCustom::getMovitEffects()
{
	QList<Effect*> list;
	list.append( new MCustomEffect( currentShader ) );
	return list;
}
