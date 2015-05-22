#ifndef THUMBNAILER_H
#define THUMBNAILER_H

#include <QThread>
#include <QImage>
#include <QMutex>

#include "profile.h"

#define ICONSIZEWIDTH 80.0
#define ICONSIZEHEIGHT 45.0



class Source;
class Frame;
class QGLWidget;



class ThumbRequest
{
public:
	enum RequestType{ PROBE, THUMB, SHADER };

	ThumbRequest( QString path )
		: typeOfRequest( PROBE ),
		caller( NULL ),
		filePath( path ),
		thumbPTS( 0 ) {}
	
	ThumbRequest( void *from, QString shader, int inputNumber )
		: typeOfRequest( SHADER ),
		caller( from ),
		inputType( inputNumber ),
		filePath( shader ),
		thumbPTS( 0 ) {}
		
	ThumbRequest( void *from, int inType, QString path, Profile prof, double pts )
		: typeOfRequest( THUMB ),
		caller( from ),
		inputType( inType ),
		filePath( path ),
		profile( prof ),
		thumbPTS( pts ) {}
	
	int typeOfRequest;
	void *caller;
	int inputType;
	QString filePath;
	Profile profile;
	double thumbPTS;
	QImage thumb;
};




class Thumbnailer : public QThread
{
	Q_OBJECT
public:
	Thumbnailer();
	~Thumbnailer();
	void setSharedContext( QGLWidget *sharedContext );
	
	bool pushRequest( ThumbRequest req );
	
protected:
	void run();
	
private slots:
	void gotResult();
	
private:	
	void probe( ThumbRequest &request );
	void makeThumb( ThumbRequest &request );
	void compileShader( ThumbRequest &request );
	
	QImage getSourceThumb( Frame *f, bool border );
	
	QList<ThumbRequest> requestList;
	QList<ThumbRequest> resultList;
	
	QMutex requestMutex, resultMutex;
	bool running;
	QGLWidget *glContext;
	
signals:
	void resultReady();
	void thumbReady( ThumbRequest );
};
#endif // THUMBNAILER_H
