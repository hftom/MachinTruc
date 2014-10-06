#ifndef PARAMETER_H
#define PARAMETER_H

#include <QVariant>
#include <QColor>



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
	enum ParameterType{ PDOUBLE, PINT, PRGBCOLOR, PRGBACOLOR };
	
	double getUnnormalizedKeyValue( int keyIndex ) {
		double range = qAbs( min.toDouble() + max.toDouble() );
		return (range * graph.keys[keyIndex].y) + min.toDouble();
	}
	
	double getNormalizedKeyValue( double val ) {
		return (val - min.toDouble()) / (max.toDouble() - min.toDouble());
	}

	QString id; // unique per filter
	QString name; // UI name
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
