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
	backupFilename = rootElement.attribute( "backup" );
	
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
	int type = InputBase::UNDEF;
	double startTime = 0;
	double duration = 0;
	qint64 size = 0;
	double fps = 0;
	int width = 0;
	int height = 0;
	double sar = 0;
	bool ilace = false;
	bool tff = true;
	int csp = 0;
	int cprim = 0;
	bool fullRange = false;
	int chromaloc = 0;
	int gamma = 0;
	QString vcodec;
	int arate = 0;
	int achannels = 0;
	int aformat = -1;
	int alayout = -1;
	QString acodec;
	QString layoutName;
	
	type = element.attribute( "type" ).toInt();
	if ( type < 1 || type >= InputBase::LAST )
		type = 0;
	duration = qMax( 1.0, element.attribute( "duration" ).toDouble() );
	startTime = element.attribute( "startTime" ).toDouble();
	fps = element.attribute( "fps" ).toDouble();
	if ( fps < 1 || fps > 3000 )
		fps = 0;
	width = element.attribute( "width" ).toInt();
	if ( width < 2 || width > MAXPROJECTWIDTH )
		width = 0;
	height = element.attribute( "height" ).toInt();
	if ( height < 2 || height > MAXPROJECTHEIGHT )
		height = 0;
	sar = element.attribute( "sar" ).toDouble();
	if ( sar < 0.5 || sar > 5 )
		sar = 0;
	ilace = element.attribute( "ilace" ).toInt() > 0;
	tff = element.attribute( "tff" ).toInt() > 0;
	csp = element.attribute( "csp" ).toInt();
	if ( csp < 1 || csp >= Profile::SPC_LAST )
		csp = 0;
	cprim = element.attribute( "cprim" ).toInt();
	if ( cprim < 1 || cprim >= Profile::PRI_LAST )
		cprim = 0;
	fullRange = element.attribute( "fullRange" ).toInt() > 0;
	chromaloc = element.attribute( "chromaloc" ).toInt();
	if ( chromaloc < 1 || chromaloc >= Profile::LOC_LAST )
		chromaloc = 0;
	gamma = element.attribute( "gamma" ).toInt();
	if ( gamma < 1 || gamma >= Profile::GAMMA_LAST )
		gamma = 0;
	vcodec = element.attribute( "vcodec" );
	arate = element.attribute( "arate" ).toInt();
	if ( arate < 1000 || arate > 256000 )
		arate = 0;
	achannels = element.attribute( "achannels" ).toInt();
	if ( achannels < 1 || achannels > 8 )
		achannels = 0;
	aformat = element.attribute( "aformat" ).toInt();
	if ( aformat < 0 || aformat >= Profile::SAMPLE_LAST )
		aformat = -1;
	alayout = element.attribute( "alayout" ).toInt();
	if ( alayout < 0 || alayout >= Profile::LAYOUT_LAST )
		alayout = -1;
	acodec = element.attribute( "acodec" );
	layoutName = element.attribute( "layoutName" );

	QDomNodeList nodes = element.childNodes();
	for ( int i = 0; i < nodes.count(); ++i ) {
		QDomElement e = nodes.at( i ).toElement();
		if ( e.isNull() )
			continue;
		
		if ( e.tagName() == "Name" )
			name = e.text();
		else if ( e.tagName() == "Size" )
			size = e.text().toLongLong();
	}
	
	Profile prof;
	prof.setStreamStartTime( startTime );
	prof.setStreamDuration( duration );
	prof.setVideoFrameRate( fps );
	if ( fps != 0 )
		prof.setVideoFrameDuration( MICROSECOND / fps );
	prof.setVideoWidth( width );
	prof.setVideoHeight( height );
	prof.setVideoSAR( sar );
	prof.setVideoInterlaced( ilace );
	prof.setVideoTopFieldFirst( tff );
	prof.setStreamStartTime( startTime );
	prof.setStreamDuration( duration );
	prof.setVideoColorSpace( csp );
	prof.setVideoColorPrimaries( cprim );
	prof.setVideoColorFullRange( fullRange );
	prof.setVideoChromaLocation( chromaloc );
	prof.setVideoGammaCurve( gamma );
	prof.setVideoCodecName( vcodec );
	if ( width == 0 || height == 0 || sar == 0 )
		prof.setHasVideo( false );
	prof.setAudioSampleRate( arate );
	prof.setAudioChannels( achannels );
	prof.setAudioFormat( aformat );
	prof.setAudioLayout( alayout );
	prof.setAudioCodecName( acodec );
	prof.setAudioLayoutName( layoutName );
	if ( arate == 0 || achannels == 0 || aformat == -1 || alayout == -1 )
		prof.setHasAudio( false );
	
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
	
	QFileInfo info( name );
	if ( !info.exists() && type != InputBase::GLSL )
		return;
	Source *source;
	if ( duration == 0 || info.size() != size || type == 0 || (!prof.hasVideo() && !prof.hasAudio()) ) {
		source = new Source( name );
	}
	else {
		source = new Source( (InputBase::InputType)type, name, prof );
	}
	sourcesList.append( source );

	for ( int i = 0; i < nodes.count(); ++i ) {
		QDomElement e = nodes.at( i ).toElement();
		if ( e.isNull() )
			continue;
		
		if ( e.tagName() == "VideoFilter" ) {
			QSharedPointer<Filter> f = XMLizer::readFilter( e, false, readError );
			if ( !f.isNull() ) {
				if ( f->getIdentifier() == "GLStabilize" ) {
					GLStabilize *stab = (GLStabilize*)f.data();
					stab->setSource( source );
				}
				source->videoFilters.append( f.staticCast<GLFilter>() );
			}
		}
		else if ( e.tagName() == "AudioFilter" ) {
			QSharedPointer<Filter> f = XMLizer::readFilter( e, true, readError );
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
	
	clip->setSpeed( speed );
	
	for ( int i = 0; i < nodes.count(); ++i ) {
		QDomElement e = nodes.at( i ).toElement();
		if ( e.isNull() )
			continue;
		
		if ( e.tagName() == "VideoFilter" ) {
			QSharedPointer<Filter> f = XMLizer::readFilter( e, false, readError );
			if ( !f.isNull() ) {
				if ( f->getIdentifier() == "GLStabilize" ) {
					GLStabilize *stab = (GLStabilize*)f.data();
					stab->setSource( clip->getSource() );
				}
				clip->videoFilters.append( f.staticCast<GLFilter>() );
			}
		}
		else if ( e.tagName() == "AudioFilter" ) {
			QSharedPointer<Filter> f = XMLizer::readFilter( e, true, readError );
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
			QSharedPointer<Filter> f = XMLizer::readFilter( e, false, readError, true );
			if ( !f.isNull() )
				clip->getTransition()->setVideoFilter( f.staticCast<GLFilter>() );
		}
		else if ( e.tagName() == "AudioFilter" ) {
			QSharedPointer<Filter> f = XMLizer::readFilter( e, true, readError, true );
			if ( !f.isNull() )
				clip->getTransition()->setAudioFilter( f.staticCast<AudioFilter>() );
		}
	}
}



bool ProjectFile::saveProject( QList<Source*> sources, Sampler *sampler, QString filename, QString backupProjectFilename )
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
	root.setAttribute( "backup",backupProjectFilename );

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
	
	
	Profile prof = source->getProfile();
	n1.setAttribute( "type", QString::number( source->getType() ) );
	n1.setAttribute( "duration", QString::number( prof.getStreamDuration(), 'e', 17 ) );
	n1.setAttribute( "startTime", QString::number( prof.getStreamStartTime(), 'e', 17 ) );
	if ( prof.hasVideo() ) {
		n1.setAttribute( "fps", QString::number( prof.getVideoFrameRate(), 'e', 17 ) );
		n1.setAttribute( "width", QString::number( prof.getVideoWidth() ) );
		n1.setAttribute( "height", QString::number( prof.getVideoHeight() ) );
		n1.setAttribute( "sar", QString::number( prof.getVideoSAR(), 'e', 17 ) );
		n1.setAttribute( "ilace", QString::number( prof.getVideoInterlaced() ) );
		n1.setAttribute( "tff", QString::number( prof.getVideoTopFieldFirst() ) );
		n1.setAttribute( "csp", QString::number( prof.getVideoColorSpace() ) );
		n1.setAttribute( "cprim", QString::number( prof.getVideoColorPrimaries() ) );
		n1.setAttribute( "fullRange", QString::number( prof.getVideoColorFullRange() ) );
		n1.setAttribute( "chromaloc", QString::number( prof.getVideoChromaLocation() ) );
		n1.setAttribute( "gamma", QString::number( prof.getVideoGammaCurve() ) );
		n1.setAttribute( "vcodec", prof.getVideoCodecName() );
	}
	if ( prof.hasAudio() ) {
		n1.setAttribute( "arate", QString::number( prof.getAudioSampleRate() ) );
		n1.setAttribute( "achannels", QString::number( prof.getAudioChannels() ) );
		n1.setAttribute( "aformat", QString::number( prof.getAudioFormat() ) );
		n1.setAttribute( "alayout", QString::number( prof.getAudioLayout() ) );
		n1.setAttribute( "acodec", prof.getAudioCodecName() );
		n1.setAttribute( "layoutName", prof.getAudioLayoutName() );
	}

	XMLizer::createText( document, n1, "Name", source->getFileName() );
	XMLizer::createInt64( document, n1, "Size", source->getSize() );
	
	for ( int i = 0; i < source->videoFilters.count(); ++i )
		XMLizer::writeFilter( document, n1, false, source->videoFilters.at( i ) );
	
	for ( int i = 0; i < source->audioFilters.count(); ++i )
		XMLizer::writeFilter( document, n1, true, source->audioFilters.at( i ) );
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
