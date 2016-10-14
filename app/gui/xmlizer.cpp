#include "xmlizer.h"



Clip* XMLizer::readClip( QDomElement &element, QList<Source*> *sourcesList, Scene *scene, bool &readError )
{
	QDomNodeList nodes = element.childNodes();
	
	QString name;
	double posInTrack = 0;
	double startTime = 0;
	double length = 0;
	double speed = 1;
	bool okName = false, okPos = false, okStart = false, okLen = false;
	Clip *clip = NULL;
	
	speed = element.attribute( "speed" ).toDouble();
	if ( speed == 0.0 )
		speed = 1.0;
	
	for ( int i = 0; i < nodes.count(); ++i ) {
		QDomElement e = nodes.at( i ).toElement();
		if ( e.isNull() )
			continue;
		
		if ( e.tagName() == "Name" ) {
			name = e.text();
			okName = true;
		}
		else if ( e.tagName() == "PosInTrack" ) {
			posInTrack = e.text().toDouble();
			okPos = true;
		}
		else if ( e.tagName() == "StartTime" ) {
			startTime = e.text().toDouble();
			okStart = true;
		}
		else if ( e.tagName() == "Length" ) {
			length = e.text().toDouble();
			okLen = true;
		}
	}
	
	if ( !( okName && okPos && okStart && okLen ) ) {
		readError = true;
		return clip;
	}
	
	// check if source exists and create clip
	for ( int i = 0; i < sourcesList->count(); ++i ) {
		if ( sourcesList->at(i)->getFileName() == name ) {
			clip = scene->createClip( sourcesList->at(i), posInTrack, startTime, length );
			break;
		}
	}
	
	if ( !clip ) {
		readError = true;
		return clip;
	}
	
	clip->setSpeed( speed );
	
	for ( int i = 0; i < nodes.count(); ++i ) {
		QDomElement e = nodes.at( i ).toElement();
		if ( e.isNull() )
			continue;
		
		if ( e.tagName() == "VideoFilter" ) {
			QSharedPointer<Filter> f = readFilter( e, false, readError );
			if ( !f.isNull() ) {
				if ( f->getIdentifier() == "GLStabilize" ) {
					GLStabilize *stab = (GLStabilize*)f.data();
					stab->setSource( clip->getSource() );
				}
				clip->videoFilters.append( f.staticCast<GLFilter>() );
			}
		}
		else if ( e.tagName() == "AudioFilter" ) {
			QSharedPointer<Filter> f = readFilter( e, true, readError );
			if ( !f.isNull() )
				clip->audioFilters.append( f.staticCast<AudioFilter>() );
		}
		else if ( e.tagName() == "Transition" ) {
			readTransition( e, clip, readError );
		}
	}
	
	return clip;
}



void XMLizer::readTransition( QDomElement &element, Clip *clip, bool &readError )
{
	QDomNodeList nodes = element.childNodes();
	
	double length = 0;
	bool okLen = false;
	
	for ( int i = 0; i < nodes.count(); ++i ) {
		QDomElement e = nodes.at( i ).toElement();
		if ( e.isNull() )
			continue;

		if ( e.tagName() == "Length" ) {
			length = e.text().toDouble();
			okLen = true;
		}
	}
	
	if ( !okLen ) {
		readError = true;
		return;
	}
	
	clip->setTransition( length );
		
	for ( int i = 0; i < nodes.count(); ++i ) {
		QDomElement e = nodes.at( i ).toElement();
		if ( e.isNull() )
			continue;
		
		if ( e.tagName() == "VideoFilter" ) {
			QSharedPointer<Filter> f = readFilter( e, false, readError, true );
			if ( !f.isNull() )
				clip->getTransition()->setVideoFilter( f.staticCast<GLFilter>() );
		}
		else if ( e.tagName() == "AudioFilter" ) {
			QSharedPointer<Filter> f = readFilter( e, true, readError, true );
			if ( !f.isNull() )
				clip->getTransition()->setAudioFilter( f.staticCast<AudioFilter>() );
		}
	}
}




QSharedPointer<Filter> XMLizer::readFilter( QDomElement &element, bool audio, bool &readError, bool transition )
{
	QDomNodeList nodes = element.childNodes();
	
	QString name;
	bool okName = false;
	QSharedPointer<Filter> filter;
	
	for ( int i = 0; i < nodes.count(); ++i ) {
		QDomElement e = nodes.at( i ).toElement();
		if ( e.isNull() )
			continue;
		
		if ( e.tagName() == "Name" ) {
			name = e.text();
			okName = true;
			break;
		}
	}
	
	if ( !okName ) {
		readError = true;
		return filter;
	}
	
	FilterCollection *fc = FilterCollection::getGlobalInstance();
	if ( transition ) {
		if ( audio ) {
			for ( int i = 0; i < fc->audioTransitions.count(); ++i ) {
				if ( fc->audioTransitions[ i ].identifier == name ) {
					filter = fc->audioTransitions[ i ].create();
					break;
				}
			}
		}
		else {
			for ( int i = 0; i < fc->videoTransitions.count(); ++i ) {
				if ( fc->videoTransitions[ i ].identifier == name ) {
					filter = fc->videoTransitions[ i ].create();
					break;
				}
			}
		}
	}
	else {
		if ( audio ) {
			for ( int i = 0; i < fc->audioFilters.count(); ++i ) {
				if ( fc->audioFilters[ i ].identifier == name ) {
					filter = fc->audioFilters[ i ].create();
					break;
				}
			}	
		}
		else {
			for ( int i = 0; i < fc->videoFilters.count(); ++i ) {
				if ( fc->videoFilters[ i ].identifier == name ) {
					filter = fc->videoFilters[ i ].create();
					break;
				}
			}
		}
	}
	
	if ( filter.isNull() ) {
		readError = true;
		return filter;
	}

	QString shader = "";
	if ( filter->getIdentifier() == "GLCustom" ) {
		for ( int i = 0; i < nodes.count(); ++i ) {
			QDomElement e = nodes.at( i ).toElement();
			if ( e.isNull() )
				continue;
			if ( e.tagName() == "Parameter" ) {
				readParameter( e, filter );
			}
		}
		
		QString shaderName = "";
		QList<Parameter*> params = filter->getParameters();
		Parameter *editor = NULL;
		for ( int i = 0; i < params.count(); ++i ) {
			if ( params[ i ]->id == "editor" ) {
				editor = params[ i ];
				// retrieve the shader name
				shaderName = editor->value.toString();
				break;
			}
		}
		
		// get the shader
		shader = ShaderCollection::getGlobalInstance()->getLocalShader( shaderName );
		if ( editor && !shader.isEmpty() ) {
			GLCustom *f = (GLCustom*)filter.data();
			// set the shader so that parameters are constructed
			// and will be reparsed in the following loop
			f->setCustomParams( shader );
		}	
	}
	
	for ( int i = 0; i < nodes.count(); ++i ) {
		QDomElement e = nodes.at( i ).toElement();
		if ( e.isNull() )
			continue;
		
		if ( e.tagName() == "PosInTrack" ) {
			filter->setPosition( e.text().toDouble() );
		}
		else if ( e.tagName() == "PosOffset" ) {
			filter->setPositionOffset( e.text().toDouble() );
		}
		else if ( e.tagName() == "Length" ) {
			filter->setLength( e.text().toDouble() );
		}
		else if ( e.tagName() == "SnapMode" ) {
			filter->setSnap( e.text().toInt() );
		}
		else if ( e.tagName() == "Parameter" ) {
			readParameter( e, filter );
		}
	}
	
	if ( filter->getIdentifier() == "GLCustom" ) {
		// restore the shader that has been overwritten
		// by the preceding loop
		QList<Parameter*> params = filter->getParameters();
		for ( int i = 0; i < params.count(); ++i ) {
			if ( params[ i ]->id == "editor" ) {
				if ( !shader.isEmpty() )
					params[ i ]->value = shader;
				else {
					params[ i ]->value = GLCustom::getDefaultShader();
				}
				break;
			}
		}
	}

	return filter;
}



void XMLizer::readParameter( QDomElement &element, QSharedPointer<Filter> f )
{
	QString type = element.attribute( "type" );
	QString name = element.attribute( "name" );
	QString value = element.attribute( "value" );
	QString hue;
	QString saturation;
	
	if ( type.isEmpty() || name.isEmpty() || value.isEmpty() )
		return;
	
	if ( type == "colorwheel" ) {
		hue = element.attribute( "hue" );
		saturation = element.attribute( "saturation" );
		if ( hue.isEmpty() || saturation.isEmpty() )
			return;
	}
	
	Parameter *p = NULL;
	QList<Parameter*> params = f->getParameters();
	for ( int i = 0; i < params.count(); ++i ) {
		if ( params[ i ]->id == name ) {
			p = params[ i ];
			break;
		}
	}
	
	if ( !p )
		return;
	
	if ( type == "double" ) {
		if ( p->type != Parameter::PDOUBLE )
			return;
		p->value = value.toDouble();
	}
	else if ( type == "inputdouble" ) {
		if ( p->type != Parameter::PINPUTDOUBLE )
			return;
		p->value = value.toDouble();
	}
	else if ( type == "int" ) {
		if ( p->type != Parameter::PINT )
			return;
		p->value = value.toInt();
	}
	else if ( type == "bool" ) {
		if ( p->type != Parameter::PBOOL )
			return;
		p->value = value.toInt();
	}
	else if ( type == "rgb" ) {
		if ( p->type != Parameter::PRGBCOLOR )
			return;
		QColor col;
		col.setNamedColor( value );
		p->value = col;
	}
	else if ( type == "rgba" ) {
		if ( p->type != Parameter::PRGBACOLOR )
			return;
		QStringList sl = value.split( "." );
		if ( sl.count() != 2 )
			return;
		QColor col;
		col.setNamedColor( sl[ 0 ] );
		col.setAlpha( sl[ 1 ].toInt() );
		p->value = col;
	}
	else if ( type == "colorwheel" ) {
		if ( p->type != Parameter::PCOLORWHEEL )
			return;
		QColor col;
		col.setRgbF( hue.toDouble(), saturation.toDouble(), value.toDouble() );
		p->value = col;
	}
	else if ( type == "string" ) {
		if ( p->type != Parameter::PSTRING )
			return;
		p->value = value.replace( QString::fromUtf8("¶"), "\n" );
	}
	else if ( type == "shader" ) {
		if ( p->type != Parameter::PSHADEREDIT )
			return;
		p->value = value;
	}
	
	p->graph.keys.clear();
		
	QDomNodeList nodes = element.childNodes();
	for ( int i = 0; i < nodes.count(); ++i ) {
		QDomElement e = nodes.at( i ).toElement();
		if ( e.isNull() )
			continue;
		
		if ( e.tagName() == "Key" ) {
			QString ktype = e.attribute( "type" );
			QString kposition = e.attribute( "position" );
			QString kvalue = e.attribute( "value" );
			
			if ( ktype.isEmpty() || kposition.isEmpty() || kvalue.isEmpty() )
				continue;
			
			int animType = AnimationKey::LINEAR;
			if ( ktype == "constant" )
				animType = AnimationKey::CONSTANT;
			if ( ktype == "curve" )
				animType = AnimationKey::CURVE;
			
			double x = kposition.toDouble();
			if ( x > 1 || x < 0 )
				continue;
			double y = kvalue.toDouble();
			if ( y > p->max.toDouble() || y < p->min.toDouble() )
				continue;
			
			int j;
			for ( j = 0; j < p->graph.keys.count(); ++j ) {
				if ( x < p->graph.keys[ j ].x )
					break;
			}
			p->graph.keys.insert( j, AnimationKey( animType, x, p->getNormalizedKeyValue( y ) ) );
		}
	}
}



void XMLizer::writeClip( QDomDocument &document, QDomNode &parent, Clip *clip )
{
	QDomElement n1 = document.createElement( "Clip" );
	parent.appendChild( n1 );

	if ( clip->getSpeed() != 1.0 )
		n1.setAttribute( "speed", QString::number( clip->getSpeed(), 'e', 17 ) );
	XMLizer::createText( document, n1, "Name", clip->sourcePath() );
	XMLizer::createDouble( document, n1, "PosInTrack", clip->position() );
	XMLizer::createDouble( document, n1, "StartTime", clip->start() );
	XMLizer::createDouble( document, n1, "Length", clip->length() );
	
	for ( int i = 0; i < clip->videoFilters.count(); ++i )
		XMLizer::writeFilter( document, n1, false, clip->videoFilters.at( i ) );
	
	for ( int i = 0; i < clip->audioFilters.count(); ++i )
		XMLizer::writeFilter( document, n1, true, clip->audioFilters.at( i ) );
	
	Transition *trans = clip->getTransition();
	if ( trans ) {
		QDomElement t = document.createElement( "Transition" );
		n1.appendChild( t );
		
		XMLizer::createDouble( document, t, "PosInTrack", trans->position() );
		XMLizer::createDouble( document, t, "Length", trans->length() );
		if ( !trans->getVideoFilter().isNull() )
			XMLizer::writeFilter( document, t, false, trans->getVideoFilter() );
		if ( !trans->getAudioFilter().isNull() )
			XMLizer::writeFilter( document, t, true, trans->getAudioFilter() );
	}
}



void XMLizer::writeFilter( QDomDocument &document, QDomNode &parent, bool audio, QSharedPointer<Filter> f )
{
	QString s = audio ? "AudioFilter" : "VideoFilter";
	QDomElement n1 = document.createElement( s );
	parent.appendChild( n1 );

	createText( document, n1, "Name", f->getIdentifier() );
	createDouble( document, n1, "PosInTrack", f->getPosition() );
	if ( f->getPositionOffset() > 0 )
		createDouble( document, n1, "PosOffset", f->getPositionOffset() );
	createDouble( document, n1, "Length", f->getLength() );
	createInt( document, n1, "SnapMode", f->getSnap() );
	
	QList<Parameter*> params = f->getParameters();
	for ( int i = 0; i < params.count(); ++i ) {
		Parameter *p = params[i];
		QDomElement pel = document.createElement( "Parameter" );
		n1.appendChild( pel );
		pel.setAttribute( "name", p->id );
		switch ( p->type ) {
			case Parameter::PDOUBLE: {
				pel.setAttribute( "type", "double" );
				pel.setAttribute( "value", QString::number( p->value.toDouble(), 'e', 17 ) );
				break;
			}
			case Parameter::PINPUTDOUBLE: {
				pel.setAttribute( "type", "inputdouble" );
				pel.setAttribute( "value", QString::number( p->value.toDouble(), 'e', 17 ) );
				break;
			}
			case Parameter::PINT: {
				pel.setAttribute( "type", "int" );
				pel.setAttribute( "value", QString::number( p->value.toInt() ) );
				break;
			}
			case Parameter::PBOOL: {
				pel.setAttribute( "type", "bool" );
				pel.setAttribute( "value", QString::number( p->value.toInt() ) );
				break;
			}
			case Parameter::PRGBCOLOR: {
				pel.setAttribute( "type", "rgb" );
				pel.setAttribute( "value", p->value.value<QColor>().name() );
				break;
			}
			case Parameter::PRGBACOLOR: {
				QColor col = p->value.value<QColor>();
				pel.setAttribute( "type", "rgba" );
				pel.setAttribute( "value", col.name() + "." + QString::number( col.alpha() ) );
				break;
			}
			case Parameter::PCOLORWHEEL: {
				QColor col = p->value.value<QColor>();
				pel.setAttribute( "type", "colorwheel" );
				pel.setAttribute( "value", QString::number( col.blueF(), 'e', 17 ) );
				pel.setAttribute( "hue", QString::number( col.redF(), 'e', 17 ) );
				pel.setAttribute( "saturation", QString::number( col.greenF(), 'e', 17 ) );
				break;
			}
			case Parameter::PSTRING: {
				pel.setAttribute( "type", "string" );
				pel.setAttribute( "value", p->value.toString().replace( "\n", QString::fromUtf8("¶") ) );
				break;
			}
			case Parameter::PSHADEREDIT: {
				pel.setAttribute( "type", "shader" );
				pel.setAttribute( "value", Parameter::getShaderName( p->value.toString() ) );
				break;
			}
			case Parameter::PSTATUS: {
				pel.setAttribute( "type", "status" );
				pel.setAttribute( "value", "" );
				break;
			}
		}
		
		for ( int i = 0; i < p->graph.keys.count(); ++i ) {
			QDomElement ke = document.createElement( "Key" );
			pel.appendChild( ke );
			QString type;
			switch ( p->graph.keys[i].keyType ) {
				case AnimationKey::CONSTANT: type = "constant"; break;
				case AnimationKey::CURVE: type = "curve"; break;
				default: type = "linear";
			}
			ke.setAttribute( "type", type );
			ke.setAttribute( "position", QString::number( p->graph.keys[i].x, 'e', 17 ) );
			ke.setAttribute( "value", QString::number( p->getUnnormalizedKeyValue( i ), 'e', 17 ) );
		}
	}
}



void XMLizer::createText( QDomDocument &document, QDomNode &parent, QString name, QString val )
{
	QDomElement e = document.createElement( name );
	QDomText t = document.createTextNode( val );
	e.appendChild( t );
	parent.appendChild( e );
}



void XMLizer::createInt( QDomDocument &document, QDomNode &parent, QString name, int val )
{
	QDomElement e = document.createElement( name );
	QDomText t = document.createTextNode( QString::number( val ) );
	e.appendChild( t );
	parent.appendChild( e );
}



void XMLizer::createInt64( QDomDocument &document, QDomNode &parent, QString name, qint64 val )
{
	QDomElement e = document.createElement( name );
	QDomText t = document.createTextNode( QString::number( val ) );
	e.appendChild( t );
	parent.appendChild( e );
}



void XMLizer::createDouble( QDomDocument &document, QDomNode &parent, QString name, double val )
{
	QDomElement e = document.createElement( name );
	QDomText t = document.createTextNode( QString::number( val, 'e', 17 ) );
	e.appendChild( t );
	parent.appendChild( e );
}
