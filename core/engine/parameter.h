#ifndef PARAMETER_H
#define PARAMETER_H

#include <QVariant>



class AnimationKey
{
public:
	enum type{ CONSTANT, LINEAR, CURVE };
	
	AnimationKey( int typeKey, double position, double value );
	
	double x, y;
	int keyType;
};



class AnimationGraph
{
public:
	double valueAt( double time );

	QList<AnimationKey> keys;
};



class Parameter
{
public:
	enum ParameterType{ PDOUBLE, PINT, PRGB };
	
	QString name;
	int type;
	QVariant min;
	QVariant defValue;
	QVariant max;
	bool keyframeable;
	QString suffix;
	
	QVariant value;
	AnimationGraph graph;
};

#endif // PARAMETER_H
