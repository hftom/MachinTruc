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



class ParamLayout
{
public:
	ParamLayout() : row(-1), column(-1), rowSpan(1), columnSpan(1) {}
	void setLayout( int r, int c, int rs, int cs ) {
		row = r;
		column = c;
		rowSpan = rs;
		columnSpan = cs;
	}
	void setLayout( int r, int c ) {
		row = r;
		column = c;
	}
	
	int row, column, rowSpan, columnSpan;
};



class Parameter
{
public:
	enum ParameterType{ PDOUBLE, PINT, PBOOL, PRGBCOLOR, PRGBACOLOR, PCOLORWHEEL, PSTRING, PINPUTDOUBLE, PSHADEREDIT };
	
	double getUnnormalizedKeyValue( int keyIndex );	
	double getNormalizedKeyValue( double val );
	
	static QList<Parameter> parseShaderParams( QString shader, int &faultyLine );
	static QString getShaderName( QString shader );

	// unique per filter
	// the following ids are reserved for OVD: xOffset, yOffset, sizePercent
	QString id;
	QString name; // UI name
	int type;
	QVariant min;
	QVariant defValue;
	QVariant max;
	bool keyframeable;
	bool hidden; // only an UI flag
	QString suffix;
	
	QVariant value;
	AnimationGraph graph;
	
	ParamLayout layout;
};

#endif // PARAMETER_H
