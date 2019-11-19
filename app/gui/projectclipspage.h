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
		QString text;
		switch (source->getType()) {
			case InputBase::GLSL:
				text = source->getDisplayName();
				break;
			default:
				text = QFileInfo( source->getFileName() ).fileName() + "\n" + QTime( hours, mins, secs ).toString("hh:mm:ss");
				break;
		}
		setText( text );
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
	
	void setPixmap( QPixmap pix ) {
		setIcon(pix);
	}
	
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
	QList<Source*> getAllSources(bool withBuiltins = true);
	QList<Source*> getBuiltinSources();
	Source* getSource( int index, const QString &filename );
	QList<Source*> getSelectedSources();
	void addSource( QPixmap pix, Source *src );
	void addBuiltin( QString name, QPixmap pix );
	
	void populateBuiltins();
	
	bool hasActiveSource() { return activeSource != NULL; }
	Source *sourceActiveSource() { return activeSource->getSource(); }
	double currentPtsActiveSource() { return activeSource->getCurrentPts(); }
	void activeSourceSetCurrentPts( double d ) { activeSource->setCurrentPts( d ); }
	Profile activeSourceGetProfile() { return activeSource->getProfile(); }

private slots:
	void sourceItemMenu( const QPoint &pos );
	void sourceItemActivated( QListWidgetItem *item, QListWidgetItem *prev );
	void showSourceProperties();
	void showSourceFilters();

signals:
	void sourceActivated();
	void openSourcesBtnClicked();
	void addSelectionToTimeline();
	void requestBuiltinThumb(QString, int);

private:
	Sampler *sampler;
	SourceListItem *activeSource;
	
	QMap<QString,QString> patternsBuiltins;
	QMap<QString,QString> titlesBuiltins;
};
#endif // PROJECTCLIPSPAGE_H
