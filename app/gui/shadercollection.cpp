#include <QDebug>

#include "engine/parameter.h"
#include "shadercollection.h"

#define MACHINTRUC_DIR "machintruc"
#define SHADER_DIR "movit"
#define LOCAL_SHADER_DIR "local"
#define SHADER_EXTENSION ".movit"



ShaderCollection ShaderCollection::globalInstance = ShaderCollection();



ShaderCollection* ShaderCollection::getGlobalInstance()
{
	return &globalInstance;
}



ShaderCollection::ShaderCollection()
{
	QDir dir;
	if ( cdLocalShaderDir( dir ) ) {
		QStringList filter;
		filter << QString( "%1%2" ).arg( "*" ).arg( SHADER_EXTENSION );
		QFileInfoList infoList = dir.entryInfoList( filter, QDir::Files, QDir::Name );
		foreach( const QFileInfo & fi, infoList ) {
			QFile f( fi.filePath() );
			if ( f.open( QIODevice::ReadOnly ) ) {
				QString shader = f.readAll();
				addShader( Parameter::getShaderName( shader ), shader );
			}
		}
	}
}



bool ShaderCollection::cdLocalShaderDir( QDir &dir )
{
	dir = QDir::home();
	if ( !dir.cd( MACHINTRUC_DIR ) ) {
		if ( !dir.mkdir( MACHINTRUC_DIR ) ) {
			qDebug() << "Can't create" << MACHINTRUC_DIR << "directory.";
			return false;
		}
		if ( !dir.cd( MACHINTRUC_DIR ) )
			return false;
	}
	if ( !dir.cd( SHADER_DIR ) ) {
		if ( !dir.mkdir( SHADER_DIR ) ) {
			qDebug() << "Can't create" << SHADER_DIR << "directory.";
			return false;
		}
		if ( !dir.cd( SHADER_DIR ) )
			return false;
	}
	if ( !dir.cd( LOCAL_SHADER_DIR ) ) {
		if ( !dir.mkdir( LOCAL_SHADER_DIR ) ) {
			qDebug() << "Can't create" << LOCAL_SHADER_DIR << "directory.";
			return false;
		}
		if ( !dir.cd( LOCAL_SHADER_DIR ) )
			return false;
	}
	return true;
}



QStringList ShaderCollection::localShadersNames()
{
	QStringList list;
	foreach( const ShaderEntry se, localShaders )
		list << se.getName();
	
	return list;	
}



QString ShaderCollection::getLocalShader( QString aName )
{
	foreach( const ShaderEntry & se, localShaders ) {
		if ( se.getName() == aName )
			return se.getShader();
	}
	return QString("");
}



bool ShaderCollection::localShaderExists( QString aName )
{
	foreach( const ShaderEntry se, localShaders ) {
		if ( se.getName() == aName )
			return true;
	}
	return false;
}



void ShaderCollection::removeShader( QString aName )
{
	for ( int i = 0; i < localShaders.count(); ++i ) {
		if ( localShaders.at( i ).getName() == aName ) {
			localShaders.takeAt( i );
			break;
		}
	}
}



void ShaderCollection::addShader( QString aName, QString aShader )
{
	for ( int i = 0; i < localShaders.count(); ++i ) {
		if ( aName < localShaders.at( i ).getName() ) {
			localShaders.insert( i, ShaderEntry( aName, aShader ) );
			return;
		}
	}
	
	localShaders.append( ShaderEntry( aName, aShader ) );
}



bool ShaderCollection::saveLocalShader( QString aName, QString aShader )
{
	QDir dir;
	if ( !cdLocalShaderDir( dir ) )
		return false;
	
	if ( !aShader.endsWith( "\n" ) )
		aShader += "\n";
	QString name = aName + SHADER_EXTENSION;
	
	QFile f( dir.filePath( name ) );
	qint64 written = 0;
	if ( f.open( QIODevice::WriteOnly | QIODevice::Truncate ) ) {
		written = f.write( aShader.toUtf8() );
		f.close();
		if ( localShaderExists( aName ) )
			removeShader( aName );
		addShader( aName, aShader );
	}
	return written > 0;
}
