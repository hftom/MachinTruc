#ifndef PROJECTCLIPSPAGE_H
#define PROJECTCLIPSPAGE_H

#include <QPixmap>
#include <QListWidgetItem>
#include <QFileInfo>
#include <QTime>
#include <QMimeData>

#include "ui_projectclipspage.h"

#include "engine/source.h"
#include "engine/cut.h"
#include "engine/sampler.h"
#include "gui/mimetypes.h"

#define THUMBHEIGHT 60.



class SourceListItem : public QListWidgetItem
{
public:
	SourceListItem( QPixmap pix, Source *src ) : QListWidgetItem(),
		source( src ) // takes ownership of src.
	{
		Profile p = source->getProfile();
		int hours, mins, secs;
		secs = p.getStreamDuration() / MICROSECOND;
		mins = secs / 60;
		secs %= 60;
		hours = mins / 60;
		mins %= 60;
		setText( QFileInfo( source->getFileName() ).fileName() + "\n" + QTime( hours, mins, secs ).toString("hh:mm:ss") );
		setIcon( pix );
		currentPts = inPoint = 0;
		outPoint = inPoint + ( p.getStreamDuration() / 2 );
	}

	const QString & getFileName() { return source->getFileName(); }
	const Profile & getProfile() { return source->getProfile(); }
	void setInPoint( QImage img, double d ) { inPoint = d; inThumb = img; }
	void setOutPoint( QImage img, double d ) { outPoint = d; outThumb = img; }
	double getInPoint() { return inPoint; }
	double getOutPoint() { return outPoint; }
	void setCurrentPts( double d ) { currentPts = d; }
	double getCurrentPts() { return currentPts; }
	Source* getSource() { return source; }
	const QImage & getInThumb() { return inThumb; }
	QSize getThumbSize() { return QSize( source->getProfile().getVideoSAR() * source->getProfile().getVideoWidth() * THUMBHEIGHT / source->getProfile().getVideoHeight(), THUMBHEIGHT ); }	
	
private:
	Source *source;
	double inPoint, outPoint;
	double currentPts;
	QImage inThumb, outThumb;
};



class ProjectSourcesPage : public QWidget, private Ui::StackProjectClips
{
	Q_OBJECT
public:
	ProjectSourcesPage( Sampler *samp );
	
	bool exists( QString name );
	void clearAllSources();
	QList<Source*> getAllSources();
	Source* getSource( int index, const QString &filename );
	void addSource( QPixmap pix, Source *src );
	
	bool hasActiveSource() { return activeSource != NULL; }
	Source *sourceActiveSource() { return activeSource->getSource(); }
	double currentPtsActiveSource() { return activeSource->getCurrentPts(); }
	void activeSourceSetCurrentPts( double d ) { activeSource->setCurrentPts( d ); }
	Profile activeSourceGetProfile() { return activeSource->getProfile(); }
	QSplitter* getSplitter() { return splitter; }

private slots:
	void sourceItemMenu( const QPoint &pos );
	void sourceItemActivated( QListWidgetItem *item, QListWidgetItem *prev );
	void showSourceProperties();
	void showSourceFilters();

signals:
	void sourceActivated();
	void openSourcesBtnClicked();
	void openBlankBtnClicked();

private:
	Sampler *sampler;
	SourceListItem *activeSource;
};
#endif // PROJECTCLIPSPAGE_H
