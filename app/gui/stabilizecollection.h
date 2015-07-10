#include <QThread>
#include <QDir>
#include <QList>
#include <QTimer>

#include "engine/source.h"



class StabilizeTransform
{
public:
	enum Status{ STABERROR, STABNOTYETREADY, STABREADY };

	StabilizeTransform( double aPts, float aX, float aY, float anAlpha, float aZoom )
		: pts(aPts), x(aX), y(aY), alpha(anAlpha), zoom(aZoom)
	{}

	double pts;
	float x;
	float y;
	float alpha;
	float zoom;
};



class StabMotionDetect : public QThread
{
public:
	StabMotionDetect( Source *aSource );
	~StabMotionDetect();
	void stop();
	// gives list ownership to the caller
	QList<StabilizeTransform>* getTransforms();
	bool getFinishedSuccess() { return finishedSuccess; }
	QString getFileName() { return source->getFileName(); }
	
protected:
	void run();
	
private:
	bool finishedSuccess;
	bool running;
	Source *source;
	QList<StabilizeTransform> *transforms;
};



class StabilizeItem
{
public:
	StabilizeItem( QString fileName, QList<StabilizeTransform> *list )
		: sourceName( fileName ),
		transforms( list ), // NULL if detection error
		refcount(0)
	{}
	~StabilizeItem() {
		if ( transforms ) {
			transforms->clear();
			delete transforms;
		}
	}
	
	QString getSourceName() const { return sourceName; }
	QList<StabilizeTransform>* getTransforms() { return transforms; }
	void use() { ++refcount; }
	bool release() { return --refcount == 0; }
	
private:
	QString sourceName;
	QList<StabilizeTransform> *transforms;
	int refcount;
};



class StabilizeCollection : public QObject
{
	Q_OBJECT
public:
	static StabilizeCollection* getGlobalInstance();
	
	QList<StabilizeTransform>* getTransforms( Source *source, int &status );
	void releaseTransforms( QList<StabilizeTransform> *list );
	
private slots:
	void checkDetectionThreads();
	
private:
	StabilizeCollection();
	StabilizeCollection( const StabilizeCollection& ) : QObject() {}

	bool cdStabilizeDir( QDir &dir );
	QString pathToFileName( QString path );

	QTimer checkDetectionTimer;
	QList<StabilizeItem*> stabItems;
	QList<StabMotionDetect*> stabDetect;
	static StabilizeCollection globalInstance;
};