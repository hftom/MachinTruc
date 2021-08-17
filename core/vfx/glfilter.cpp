#include "glfilter.h"


QString GLFilter::getQStringShader(QString name)
{
	QString data;
	QFile file(QString(":/my_shaders/shaders/%1").arg(name));
	
	if(file.open(QIODevice::ReadOnly)) {
    	data = file.readAll();
		file.close();
	}

	return data;
}



std::string GLFilter::getShader(QString name)
{
	return GLFilter::getQStringShader(name).toStdString();
}
