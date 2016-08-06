#include <QThread>
#include <QDir>
#include <QList>
#include <QTimer>
#include <QDateTime>

#include "source.h"



class StabilizeTransform
{
public:
	enum Status{ STABERROR, STABENQUEUED, STABINPROGRESS, STABREADY };

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
	void go();
	void stop();
	// gives list ownership to the caller
	QList<StabilizeTransform>* getTransformsOwnership();
	bool getFinishedSuccess() { return finishedSuccess; }
	bool getStarted() { return started; }
	int getProgress() { return progress; }
	QString getFileName() { return source->getFileName(); }
	void setLastQuery() { lastQuery = QDateTime::currentDateTime(); }
	bool isOutDatedQuery() { return lastQuery.secsTo(QDateTime::currentDateTime()) > 10; }
	
protected:
	void run();
	
private:
	int progress;
	bool started;
	bool finishedSuccess;
	bool running;
	Source *source;
	QList<StabilizeTransform> *transforms;
	QDateTime lastQuery;
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
	
	QList<StabilizeTransform>* getTransforms( Source *source, int &status, int &progress );
	void releaseTransforms( QList<StabilizeTransform> *list );
	bool hasTransforms( QString fileName );
	static bool cdStabilizeDir( QDir &dir );
	static QString pathToFileName( QString path );
	
private slots:
	void checkDetectionThreads();
	
private:
	StabilizeCollection();
	StabilizeCollection( const StabilizeCollection& ) : QObject() {}

	QTimer checkDetectionTimer;
	QList<StabilizeItem*> stabItems;
	QList<StabMotionDetect*> stabDetect;
	static StabilizeCollection globalInstance;
};
