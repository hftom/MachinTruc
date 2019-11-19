#include <QFileInfo>

#include "engine/source.h"



Source::Source( InputBase::InputType t, QString path, Profile prof, QString nameToDisplay ) 
	: fileName( path ),
	displayName(nameToDisplay),
	profile( prof ),
	type( t ),
	refcount( 1 )
{
	size = QFileInfo( path ).size();
}



Source::Source( QString path )
	: fileName( path ),
	type( InputBase::UNDEF ),
	refcount( 1 )
{
	size = QFileInfo( path ).size();
}



void Source::use()
{
	++refcount;
}



void Source::release()
{
	if (--refcount)
		return;
	delete this;
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



const QString & Source::getDisplayName() const
{
	return displayName.isEmpty() ? fileName : displayName;
}



qint64 Source::getSize() const
{
	return size;
}



const Profile & Source::getProfile() const
{
	return profile;
}



InputBase::InputType Source::getType() const
{
	return type;
}
