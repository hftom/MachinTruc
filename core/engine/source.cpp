#include "engine/source.h"



Source::Source( InputBase::InputType t, QString path, Profile prof ) 
{
	type = t;
	fileName = path;
	profile = prof;
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
