#ifndef FILTER_H
#define FILTER_H

#include <QMutex>
#include <QString>
#include <QList>
#include <QSharedPointer>

#include "parameter.h"



class FilterTransform
{
public:
	enum TransformType{ NERATIO = 1, SCALE = 2, TRANSLATE = 4, ROTATE = 8 };
	FilterTransform( int type, double val1, double val2 = 0 )
		: transformType( type ),
		v1( val1 ),
		v2( val2 )
	{}
	
	int transformType;
	double v1, v2;
};



class Filter : public QObject // for easier translations
{
public:
	enum snapType{ SNAPALL, SNAPNONE, SNAPSTART, SNAPEND };

	Filter( QString id, QString name );
	virtual ~Filter();
	
	virtual QList<Parameter*> getParameters() { return parameters; }
	void splitParameters( Filter *second, double posPts );

	virtual void setPosition( double p ) { posInTrack = p; }
	virtual double getPosition() { return posInTrack; }
	virtual void setPositionOffset( double p ) { posOffset = p; }
	virtual double getPositionOffset() { return posOffset; }
	virtual void setLength( double len ) { length = len; }
	virtual double getLength() { return length; }
	int getSnap() { return snap; }
	void setSnap( int s ) { snap = s; }
	
	QString getFilterName() { return filterName; }
	QString getIdentifier() { return identifier; }
	
	virtual void ovdUpdate( QString /*type*/, QVariant /*val*/ ) {}
	void enableOVD( bool b ) { showOVD = b; }
	bool ovdEnabled() { return showOVD; }

protected:	
	Parameter* addParameter( QString id, QString name, int type, QVariant def, QVariant min, QVariant max, bool keyframeable, const QString &suffix = QString() );
	Parameter* addBooleanParameter( QString id, QString name, QVariant def );
	QVariant getParamValue( Parameter *param, double pts = 0 );
	double getNormalizedTime( double pts );
	
private:
	QList<Parameter*> parameters;
	QString identifier, filterName;
	double posInTrack, length;
	double posOffset;
	int snap;
	bool showOVD;
};

#endif // FILTER_H
