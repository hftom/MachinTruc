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



class CutListItem
{
public:
	CutListItem( Cut *c, QImage img )
		: cut( c ),
		thumb( img )
	{
		currentPts = cut->getStart();
	}
	
	const QImage & getThumb() { return thumb; }
	QString getFileName() { return cut->getSource()->getFileName(); }
	Cut* getCut() { return cut; }
	
private:
	Cut *cut;
	double currentPts;
	QImage thumb;
};



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
	Cut* getCut( int index, QString filename ) {
		if ( filename != source->getFileName() )
			return NULL;
		if ( index < 0 || index > cutList.count() - 1 )
			return NULL;
		return cutList.at( index )->getCut();
	}
	
	void addCut() {
		int insert = 0;
		int i = cutList.count() - 1;
		while ( i > -1 ) {
			if ( inPoint >= cutList.at( i )->getCut()->getStart() ) {
				insert = i + 1;
				break;
			}
			--i;
		}
		cutList.insert( insert, new CutListItem( new Cut( source, inPoint, outPoint - inPoint ), inThumb ) );
		printf("addCut : inPoint=%f outPoint=%f len=%f frames=%f\n", inPoint, outPoint, outPoint - inPoint, (outPoint - inPoint) / source->getProfile().getVideoFrameDuration());
	}
	
	QList<CutListItem*> *getCutList() { return &cutList; }
	
private:
	Source *source;
	double inPoint, outPoint;
	double currentPts;
	QImage inThumb, outThumb;
	QList<CutListItem*> cutList;
};



class CutListModel : public QAbstractListModel
{
	Q_OBJECT
public:
	CutListModel() : QAbstractListModel(),
		source( NULL )
	{
	}
	int rowCount( const QModelIndex &parent = QModelIndex() ) const {
		Q_UNUSED( parent );
		return source ? source->getCutList()->count() : 0;
	}
	
	QVariant data( const QModelIndex &index, int role=Qt::DisplayRole ) const {
		if ( !source || !index.isValid() )
			return QVariant();
		if ( role == Qt::DecorationRole )
			return QVariant( source->getCutList()->at( index.row() )->getThumb() );
		if ( role == Qt::DisplayRole ) {
			int hours, mins, secs;
			double u = source->getCutList()->at( index.row() )->getCut()->getLength();
			secs = u / MICROSECOND;
			u -= (double)secs * MICROSECOND;
			u /= source->getCutList()->at( index.row() )->getCut()->getSource()->getProfile().getVideoFrameDuration();
			mins = secs / 60;
			secs %= 60;
			hours = mins / 60;
			mins %= 60;
			return QVariant( QTime( hours, mins, secs ).toString("hh:mm:ss") + "." + QString::number( (int)(u + 0.5) ) );
		}
		return QVariant();
	}
	
	Qt::DropActions supportedDropActions() const {
		return Qt::CopyAction;
	}
	
	Qt::ItemFlags flags( const QModelIndex &index ) const {
		Qt::ItemFlags defaultFlags = QAbstractListModel::flags( index );
		if ( index.isValid() )
			return Qt::ItemIsDragEnabled | defaultFlags;
		return defaultFlags;
	}
	
	QStringList mimeTypes() const {
		QStringList types;
		types << MIMETYPECUT;
		return types;
	}
	
	QMimeData* mimeData(const QModelIndexList &indexes) const {
		QMimeData *mimeData = new QMimeData();
		QByteArray encodedData;
		foreach ( const QModelIndex &index, indexes ) {
			if ( index.isValid() ) {
				encodedData.append( QString::number( index.row() ) + " " );
				encodedData.append( source->getCutList()->at( index.row() )->getFileName() );
			}
		}
		mimeData->setData( MIMETYPECUT, encodedData );
		return mimeData;
	}
 
	void setSource( SourceListItem* src ) { source = src; }
		
private:
	SourceListItem *source;
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
	Cut *activeSourceGetCut( int index, QString filename ) {
		return activeSource->getCut( index, filename ); }
	void activeSourceSetCurrentPts( double d ) { activeSource->setCurrentPts( d ); }
	Profile activeSourceGetProfile() { return activeSource->getProfile(); }
	
	
	
public slots:
	void newCut( SourceListItem* );

private slots:
	void sourceItemMenu( const QPoint &pos );
	void sourceItemActivated( QListWidgetItem *item, QListWidgetItem *prev );
	void showSourceProperties();
	void showSourceFilters();

signals:
	void sourceActivated();
	void openSourcesBtnClicked();

private:
	CutListModel model;
	Sampler *sampler;
	SourceListItem *activeSource;
};
#endif // PROJECTCLIPSPAGE_H
