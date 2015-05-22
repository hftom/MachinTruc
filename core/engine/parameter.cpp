#include <QDebug>

#include "util.h"
#include "parameter.h"


	
double Parameter::getUnnormalizedKeyValue( int keyIndex )
{
	double range = qAbs( -min.toDouble() + max.toDouble() );
	return (range * graph.keys[keyIndex].y) + min.toDouble();
}



double Parameter::getNormalizedKeyValue( double val )
{
	return (val - min.toDouble()) / (max.toDouble() - min.toDouble());
}



QList<Parameter> Parameter::parseShaderParams( QString shader, int &faultyLine ) // static
{
	QList<Parameter> list;
	faultyLine = -1;
	QStringList sl = shader.split( "\n" );
	for ( int i = 0; i < sl.count(); ++i ) {
		QString line = sl[i];
		if ( line.trimmed().startsWith( "//PARAM" ) ) {
			Parameter p;
			QStringList params = line.trimmed().split( ":" );
			if ( params.count() == 8 ) {
				p.id = params[1].trimmed();
				p.name = params[2].trimmed();
				if ( params[3].trimmed() == "float" )
					p.type = Parameter::PDOUBLE;
				else {
					faultyLine = i + 1;
					return list;
				}
				bool ok1, ok2, ok3;
				p.defValue = params[4].trimmed().toDouble( &ok1 );
				p.min = params[5].trimmed().toDouble( &ok2 );
				p.max = params[6].trimmed().toDouble( &ok3 );
				if ( !ok1 || !ok2 || !ok3 ) {
					faultyLine = i + 1;
					return list;
				}
				if ( params[7].trimmed().toUpper() == "TRUE" )
					p.keyframeable = true;
				else if ( params[7].trimmed().toUpper() == "FALSE" )
					p.keyframeable = false;
				else {
					faultyLine = i + 1;
					return list;
				}
				list.append( p );
			}
			else if ( params.count() == 5 ) {
				p.id = params[1].trimmed();
				p.name = params[2].trimmed();
				if ( params[3].trimmed() == "rgb" ) {
					p.type = Parameter::PRGBCOLOR;
					QString color = params[4].trimmed();
					if ( color.length() == 6 ) {
						bool ok;
						color.toUInt( &ok, 16 );
						if ( !ok ) {
							faultyLine = i + 1;
							return list;
						}
						QColor col;
						color.prepend( "#" );
						col.setNamedColor( color );
						p.defValue = col;
					}
					else {
						faultyLine = i + 1;
						return list;
					}
				}
				else if ( params[3].trimmed() == "rgba" ) {
					p.type = Parameter::PRGBACOLOR;
					QString color = params[4].trimmed();
					if ( color.length() == 8 ) {
						bool ok;
						color.toUInt( &ok, 16 );
						if ( !ok ) {
							faultyLine = i + 1;
							return list;
						}
						QColor col;
						col.setNamedColor( color.left( 6 ).prepend( "#" ) );
						col.setAlpha( color.right( 2 ).toUInt( &ok, 16 ) );
						p.defValue = col;
					}
					else {
						faultyLine = i + 1;
						return list;
					}
				}
				else {
					faultyLine = i + 1;
					return list;
				}
				list.append( p );
			}
			else {
				faultyLine = i + 1;
				return list;
			}
		}
	}
	return list;
}



AnimationKey::AnimationKey( int typeKey, double position, double value )
{
	keyType = typeKey;
	x = position;
	y = value;
}



double AnimationGraph::valueAt( double time )
{
	int i;
	for ( i = keys.count() - 1; i > -1; --i ) {
		if ( keys[i].x <= time ) {
			AnimationKey k1 = keys[i];
			if ( i == keys.count() - 1 )
				return k1.y;
				
			AnimationKey k2 = keys[i+1];
			switch ( k1.keyType ) {
				case AnimationKey::LINEAR:
					return linearInterpolate( k1.y, k2.y, ( time - k1.x ) / ( k2.x - k1.x ) );
				case AnimationKey::CURVE:
					return cosineInterpolate( k1.y, k2.y, ( time - k1.x ) / ( k2.x - k1.x ) );
				default:
					return k1.y;
			}
		}
	}
	return keys[0].y;
}
