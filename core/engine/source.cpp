#include "engine/source.h"



Source::Source( InputBase::InputType t, QString path, Profile prof ) 
	: fileName( path ),
	profile( prof ),
	type( t )
{
}



Source::Source( QString path )
	: fileName( path ),
	type( InputBase::UNDEF )
{
}



void Source::setAfter( InputBase::InputType t, Profile prof )
{
	profile = prof;
	type = t;
}



const QString & Source::getFileName() const
{
	return fileName;
}



const Profile & Source::getProfile() const
{
	return profile;
}



InputBase::InputType Source::getType() const
{
	return type;
}
