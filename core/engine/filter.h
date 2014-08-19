#ifndef FILTER_H
#define FILTER_H

#include <QMutex>
#include <QString>
#include <QList>

#include "parameter.h"



class Filter : public QObject // for easier translations
{
public:
	Filter( QString id, QString name );
	virtual ~Filter();
	void use();
	void release();
	
	virtual QList<Parameter*> getParameters() { return parameters; }

	virtual void setPosition( double p ) { posInTrack = p; }
	virtual double getPosition() { return posInTrack; }
	virtual void setLength( double len ) { length = len; }
	virtual double getlength() { return length; }
	
	QString getFilterName() { return filterName; }
	QString getIdentifier() { return identifier; }

protected:	
	Parameter* addParameter( QString name, int type, QVariant def, QVariant min, QVariant max, bool keyframeable, const QString &suffix = QString() );
	QVariant getParamValue( Parameter *param, double pts = 0 );
	
private:
	QList<Parameter*> parameters;
	QString filterName, identifier;
	double posInTrack, length;
	
	int refCount;
	QMutex rcMutex;
};

#endif // FILTER_H
