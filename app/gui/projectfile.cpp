#include "engine/filtercollection.h"

#include "projectfile.h"



bool ProjectFile::loadProject( QString filename )
{
	document = QDomDocument();
	QFile file( filename );
	if ( !file.open( QIODevice::ReadOnly ) ) {
		qDebug() << "can't open file.";
		return false;;
	}
	if ( !document.setContent( &file ) ) {
		file.close();
		qDebug() << "can't set document content.";
		return false;
	}
	file.close();

	QDomElement rootElement = document.documentElement();
	if ( rootElement.tagName() != "MachinTruc" ) {
		qDebug() << "root != MachinTruc" << rootElement.tagName();
		return false;
	}
	
	QString s = rootElement.attribute( "width" );
	if ( s.isEmpty() || s.toInt() < 1 || s.toInt() > MAXPROJECTWIDTH )
		return false;
	projectProfile.setVideoWidth( s.toInt() );
	s = rootElement.attribute( "height" );
	if ( s.isEmpty() || s.toInt() < 1 || s.toInt() > MAXPROJECTHEIGHT )
		return false;
	projectProfile.setVideoHeight( s.toInt() );
	s = rootElement.attribute( "sar" );
	if ( s.isEmpty() || s.toDouble() < 0.3 || s.toDouble() >= 2 )
		return false;
	projectProfile.setVideoSAR( s.toDouble() );
	s = rootElement.attribute( "fps" );
	if ( s.isEmpty() || s.toDouble() < 5 || s.toDouble() > 120 )
		return false;
	projectProfile.setVideoFrameRate( s.toDouble() );
	projectProfile.setVideoFrameDuration( MICROSECOND / s.toDouble() );
	s = rootElement.attribute( "interlace" );
	if ( s.isEmpty() )
		return false;
	projectProfile.setVideoInterlaced( s.toInt() );
	s = rootElement.attribute( "tff" );
	if ( s.isEmpty() )
		return false;
	projectProfile.setVideoTopFieldFirst( s.toInt() );
	s = rootElement.attribute( "samplerate" );
	if ( s.isEmpty() || s.toInt() < 1000 || s.toInt() > 192000 )
		return false;
	//projectProfile.setAudioSampleRate( s.toInt() );
	s = rootElement.attribute( "channels" );
	if ( s.isEmpty() || s.toInt() < 2 || s.toInt() > 8 )
		return false;
	//projectProfile.setAudioChannels( s.toInt() );
	s = rootElement.attribute( "layout" );
	if ( s.isEmpty() )
		return false;
	//projectProfile.setAudioLayout( s.toInt() );
	
	QDomNodeList nodes = rootElement.childNodes();
	
	// get all "Source"
	for ( int i = 0; i < nodes.count(); ++i ) {
		QDomElement e = nodes.at( i ).toElement();
		if ( e.isNull() )
			continue;
		
		if ( e.tagName() == "Source" ) {
			readSource( e );
		}
	}
	
	// get all "Scene"
	for ( int i = 0; i < nodes.count(); ++i ) {
		QDomElement e = nodes.at( i ).toElement();
		if ( e.isNull() )
			continue;
		
		if ( e.tagName() == "Scene" ) {
			readScene( e );
		}
	}
	
	if ( !sceneList.count() ) {
		while ( !sourcesList.isEmpty() )
			delete sourcesList.takeFirst();
		return false;
	}
	
	return true;
}



void ProjectFile::readSource( QDomElement &element )
{
	QString name;
	double startTime = 0;
	double duration = 0;
	
	QDomNodeList nodes = element.childNodes();
	for ( int i = 0; i < nodes.count(); ++i ) {
		QDomElement e = nodes.at( i ).toElement();
		if ( e.isNull() )
			continue;
		
		if ( e.tagName() == "Name" )
			name = e.text();
		else if ( e.tagName() == "StartTime" )
			startTime = e.text().toDouble();
		else if ( e.tagName() == "Duration" )
			duration = e.text().toDouble();
	}
	
	if ( name.isEmpty() ) {
		readError = true;
		return;
	}
	
	// sourcesList must _not_ have duplicates.
	for ( int i = 0; i < sourcesList.count(); ++i ) {
		if ( sourcesList[i]->getFileName() == name ) {
			readError = true;
			return;
		}
	}

	Source *source = new Source( name );
	sourcesList.append( source );

	for ( int i = 0; i < nodes.count(); ++i ) {
		QDomElement e = nodes.at( i ).toElement();
		if ( e.isNull() )
			continue;
		
		if ( e.tagName() == "VideoFilter" ) {
			QSharedPointer<Filter> f = readFilter( e, false );
			if ( !f.isNull() )
				source->videoFilters.append( f.staticCast<GLFilter>() );
		}
		else if ( e.tagName() == "AudioFilter" ) {
			QSharedPointer<Filter> f = readFilter( e, true );
			if ( !f.isNull() )
				source->audioFilters.append( f.staticCast<AudioFilter>() );
		}
	}
}



void ProjectFile::readScene( QDomElement &element )
{
	Scene *scene = new Scene( projectProfile );
	sceneList.append( scene );

	QDomNodeList nodes = element.childNodes();
	
	for ( int i = 0; i < nodes.count(); ++i ) {
		QDomElement e = nodes.at( i ).toElement();
		if ( e.isNull() )
			continue;

		if ( e.tagName().startsWith( "Track" ) ) {
			QString s = e.tagName().remove( "Track" );
			bool ok;
			int index = s.toInt( &ok );
			if ( ok )
				readTrack( e, scene, index );
		}
	}
}



void ProjectFile::readTrack( QDomElement &element, Scene *scene, int index )
{	
	for ( int i = scene->tracks.count(); i <= index; ++i )
		scene->tracks.append( new Track() );
	
	QDomNodeList nodes = element.childNodes();
	
	for ( int i = 0; i < nodes.count(); ++i ) {
		QDomElement e = nodes.at( i ).toElement();
		if ( e.isNull() )
			continue;
		
		if ( e.tagName() == "Clip" )
			readClip( e, scene, index );
	}
}



void ProjectFile::readClip( QDomElement &element, Scene *scene, int trackIndex )
{
	QDomNodeList nodes = element.childNodes();
	
	QString name;
	double posInTrack = 0;
	double startTime = 0;
	double length = 0;
	bool okName = false, okPos = false, okStart = false, okLen = false;
	Clip *clip = NULL;
	
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
		return;
	}
	
	// check if source exists and create clip
	for ( int i = 0; i < sourcesList.count(); ++i ) {
		if ( sourcesList[ i ]->getFileName() == name ) {
			clip = scene->createClip( sourcesList[ i ], posInTrack, startTime, length );
			break;
		}
	}
	
	if ( !clip ) {
		readError = true;
		return;
	}
	
	for ( int i = 0; i < nodes.count(); ++i ) {
		QDomElement e = nodes.at( i ).toElement();
		if ( e.isNull() )
			continue;
		
		if ( e.tagName() == "VideoFilter" ) {
			QSharedPointer<Filter> f = readFilter( e, false );
			if ( !f.isNull() )
				clip->videoFilters.append( f.staticCast<GLFilter>() );
		}
		else if ( e.tagName() == "AudioFilter" ) {
			QSharedPointer<Filter> f = readFilter( e, true );
			if ( !f.isNull() )
				clip->audioFilters.append( f.staticCast<AudioFilter>() );
		}
		else if ( e.tagName() == "Transition" ) {
			readTransition( e, clip );
		}
	}
	
	if ( !scene->canMove( clip, length, posInTrack, trackIndex ) ) {
		delete clip;
		readError = true;
		return;
	}
	else {
		scene->addClip( clip, trackIndex );
	}
}



void ProjectFile::readTransition( QDomElement &element, Clip *clip )
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
			QSharedPointer<Filter> f = readFilter( e, false, true );
			if ( !f.isNull() )
				clip->getTransition()->setVideoFilter( f.staticCast<GLFilter>() );
		}
		else if ( e.tagName() == "AudioFilter" ) {
			QSharedPointer<Filter> f = readFilter( e, true, true );
			if ( !f.isNull() )
				clip->getTransition()->setAudioFilter( f.staticCast<AudioFilter>() );
		}
	}
}



QSharedPointer<Filter> ProjectFile::readFilter( QDomElement &element, bool audio, bool transition )
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
	
	return filter;
}



void ProjectFile::readParameter( QDomElement &element, QSharedPointer<Filter> f )
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
		qDebug() << p->value.toString();
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



bool ProjectFile::saveProject( QList<Source*> sources, Sampler *sampler, QString filename )
{
	document =  QDomDocument( "MachinTrucVideoEdit" );
	QDomElement root = document.createElement( "MachinTruc" );
	document.appendChild( root );
	
	Profile prof = sampler->getCurrentScene()->getProfile();
	root.setAttribute( "width", QString::number( prof.getVideoWidth() ) );
	root.setAttribute( "height", QString::number( prof.getVideoHeight() ) );
	root.setAttribute( "sar", QString::number( prof.getVideoSAR(), 'e', 17 ) );
	root.setAttribute( "fps", QString::number( prof.getVideoFrameRate(), 'e', 17 ) );
	root.setAttribute( "interlace", QString::number( prof.getVideoInterlaced() ) );
	root.setAttribute( "tff", QString::number( prof.getVideoTopFieldFirst() ) );
	root.setAttribute( "samplerate", QString::number( prof.getAudioSampleRate() ) );
	root.setAttribute( "channels", QString::number( prof.getAudioChannels() ) );
	root.setAttribute( "layout", QString::number( prof.getAudioLayout() ) );

	for ( int i = 0; i < sources.count(); ++i )
		writeSource( root, sources[i] );
	writeTracks( root, sampler );
	
	QDomNode proc = document.createProcessingInstruction( "xml","version=\"1.0\" encoding=\"UTF-8\"" );
	document.insertBefore( proc, document.firstChild() );
	
	QFile data( filename );
	if ( data.open( QFile::WriteOnly | QFile::Truncate ) ) {
		QTextStream out( &data );
		document.save( out, 2 );
		return true;
	}
	
	return false;
}



void ProjectFile::writeSource( QDomNode &parent, Source *source )
{
	QDomElement n1 = document.createElement( "Source" );
	parent.appendChild( n1 );

	createText( n1, "Name", source->getFileName() );
	createDouble( n1, "StartTime", source->getProfile().getStreamStartTime() );
	createDouble( n1, "Duration", source->getProfile().getStreamDuration() );
	
	for ( int i = 0; i < source->videoFilters.count(); ++i )
		writeFilter( n1, false, source->videoFilters.at( i ) );
	
	for ( int i = 0; i < source->audioFilters.count(); ++i )
		writeFilter( n1, true, source->audioFilters.at( i ) );
}



void ProjectFile::writeTracks( QDomNode &parent, Sampler *sampler )
{
	QList<Scene*> list = sampler->getSceneList();
	for ( int j = 0; j < list.count(); ++j ) {
		QDomElement n = document.createElement( "Scene" );
		parent.appendChild( n );
		Scene *scene = list[j];
		for ( int i = 0; i < scene->tracks.count(); ++i ) {
			QDomElement n1 = document.createElement( "Track" + QString::number( i ) );
			n.appendChild( n1 );
			Track *track = scene->tracks[i];
			for ( int j = 0; j < track->clipCount(); ++j )
				writeClip( n1, track->clipAt( j ) );
		}
	}
}



void ProjectFile::writeClip( QDomNode &parent, Clip *clip )
{
	QDomElement n1 = document.createElement( "Clip" );
	parent.appendChild( n1 );

	createText( n1, "Name", clip->sourcePath() );
	createDouble( n1, "PosInTrack", clip->position() );
	createDouble( n1, "StartTime", clip->start() );
	createDouble( n1, "Length", clip->length() );
	
	for ( int i = 0; i < clip->videoFilters.count(); ++i )
		writeFilter( n1, false, clip->videoFilters.at( i ) );
	
	for ( int i = 0; i < clip->audioFilters.count(); ++i )
		writeFilter( n1, true, clip->audioFilters.at( i ) );
	
	Transition *trans = clip->getTransition();
	if ( trans ) {
		QDomElement t = document.createElement( "Transition" );
		n1.appendChild( t );
		
		createDouble( t, "PosInTrack", trans->position() );
		createDouble( t, "Length", trans->length() );
		if ( !trans->getVideoFilter().isNull() )
			writeFilter( t, false, trans->getVideoFilter() );
		if ( !trans->getAudioFilter().isNull() )
			writeFilter( t, true, trans->getAudioFilter() );
	}
}



void ProjectFile::writeFilter( QDomNode &parent, bool audio, QSharedPointer<Filter> f )
{
	QString s = audio ? "AudioFilter" : "VideoFilter";
	QDomElement n1 = document.createElement( s );
	parent.appendChild( n1 );

	createText( n1, "Name", f->getIdentifier() );
	createDouble( n1, "PosInTrack", f->getPosition() );
	if ( f->getPositionOffset() > 0 )
		createDouble( n1, "PosOffset", f->getPositionOffset() );
	createDouble( n1, "Length", f->getLength() );
	createInt( n1, "SnapMode", f->getSnap() );
	
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



void ProjectFile::createText( QDomNode &parent, QString name, QString val )
{
	QDomElement e = document.createElement( name );
	QDomText t = document.createTextNode( val );
	e.appendChild( t );
	parent.appendChild( e );
}



void ProjectFile::createInt( QDomNode &parent, QString name, int val )
{
	QDomElement e = document.createElement( name );
	QDomText t = document.createTextNode( QString::number( val ) );
	e.appendChild( t );
	parent.appendChild( e );
}



void ProjectFile::createDouble( QDomNode &parent, QString name, double val )
{
	QDomElement e = document.createElement( name );
	QDomText t = document.createTextNode( QString::number( val, 'e', 17 ) );
	e.appendChild( t );
	parent.appendChild( e );
}
