#include "engine/clip.h"
#include "engine/util.h"



Clip::Clip( Source *src, double posInTrackPTS, double strt, double len )
	: source( src ),
	posInTrack( posInTrackPTS ),
	clipStart( strt ),
	clipLength( len ),
	frameDuration( MICROSECOND / 25.0 ),
	in( NULL ),
	transition( NULL ),
	speed( 1 )
{
}



Clip::~Clip()
{
	if ( in )
		in->setUsed( false );
	if ( transition )
		delete transition;
}



void Clip::setPosition( double p )
{
	posInTrack = p;
	
	for ( int i = 0; i< videoFilters.count(); ++i )
		videoFilters.at( i )->setPosition( posInTrack );
	for ( int i = 0; i< audioFilters.count(); ++i )
		audioFilters.at( i )->setPosition( posInTrack );
	if ( transition )
		transition->setPosition( posInTrack );
}



void Clip::newLength( Filter *f )
{
	switch ( f->getSnap() ) {
		case Filter::SNAPSTART: {
			if ( f->getLength() > length() )
				f->setLength( length() );
			break;
		}
		case Filter::SNAPEND: {
			double npos = length() - f->getLength();
			if ( npos < 0 ) {
				f->setPositionOffset( 0 );
				f->setLength( length() );
			}
			else
				f->setPositionOffset( npos );
			break;
		}
		case Filter::SNAPNONE: {
			if ( f->getPositionOffset() + f->getLength() > length() ) {
				double npos = length() - f->getLength();
				if ( npos < 0 ) {
					f->setPositionOffset( 0 );
					f->setLength( length() );
				}
				else
					f->setPositionOffset( npos );
			}
			break;
		}
		default:
			f->setLength( length() );
	}
}



void Clip::setLength( double len )
{
	clipLength = len;
	
	for ( int i = 0; i< videoFilters.count(); ++i )
		newLength( videoFilters.at( i ).data() );
	for ( int i = 0; i< audioFilters.count(); ++i )
		newLength( audioFilters.at( i ).data() );
}



double Clip::length()
{
	return nearestPTS( clipLength, frameDuration );
}



void Clip::setFrameDuration( double d )
{
	frameDuration = d;
	for ( int i = 0; i< videoFilters.count(); ++i ) {
		Filter *f = videoFilters.at( i ).data();
		f->setLength( nearestPTS( f->getLength(), frameDuration ) );
		newLength( f );
	}
	for ( int i = 0; i< audioFilters.count(); ++i ) {
		Filter *f = audioFilters.at( i ).data();
		f->setLength( nearestPTS( f->getLength(), frameDuration ) );
		newLength( f );
	}
	if ( transition )
		transition->setFrameDuration( frameDuration );
}



void Clip::setInput( InputBase *i )
{
	if ( in && i != in )
		in->setUsed( false );
	in = i;
	if ( in )
		in->setUsed( true );
}



void Clip::setTransition( double len )
{
	if ( !transition )
		transition = new Transition( posInTrack, len );
	transition->setPosition( posInTrack );
	transition->setLength( len );
	transition->setFrameDuration( frameDuration );
}



void Clip::setTransition( Transition *trans )
{
	if (transition) {
		delete transition;
	}
	transition = trans;
}



void Clip::removeTransition()
{
	if ( transition )
		delete transition;
	transition = NULL;
}
