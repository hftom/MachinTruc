#ifndef FILTER_H
#define FILTER_H

#include <QMutex>
#include <QString>
#include <QList>
#include <QSharedPointer>

#include "parameter.h"



class Filter : public QObject // for easier translations
{
public:
	Filter( QString id, QString name );
	virtual ~Filter();
	
	virtual QList<Parameter*> getParameters() { return parameters; }
	void splitParameters( Filter *second, double posPts );

	virtual void setPosition( double p ) { posInTrack = p; }
	virtual double getPosition() { return posInTrack; }
	virtual void setLength( double len ) { length = len; }
	virtual double getLength() { return length; }
	
	QString getFilterName() { return filterName; }
	QString getIdentifier() { return identifier; }

protected:	
	Parameter* addParameter( QString id, QString name, int type, QVariant def, QVariant min, QVariant max, bool keyframeable, const QString &suffix = QString() );
	Parameter* addBooleanParameter( QString id, QString name, QVariant def );
	QVariant getParamValue( Parameter *param, double pts = 0 );
	double getNormalizedTime( double pts );
	
private:
	QList<Parameter*> parameters;
	QString identifier, filterName;
	double posInTrack, length;
};

#endif // FILTER_H
