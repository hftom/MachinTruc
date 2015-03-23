#ifndef EFFECTLISTVIEW_H
#define EFFECTLISTVIEW_H

#include <QAbstractListModel>
#include <QListView>
#include <QImage>
#include <QMimeData>
#include <QDrag>
#include <QPainter>

#include "engine/filtercollection.h"
#include "gui/mimetypes.h"



class EffectListModel : public QAbstractListModel
{
	Q_OBJECT
public:
	EffectListModel( QList<FilterEntry> *f ) : QAbstractListModel()
	{
		filters = f;
	}
	int rowCount( const QModelIndex &parent = QModelIndex() ) const {
		Q_UNUSED( parent );
		return filters->count();
	}
	
	QVariant data( const QModelIndex &index, int role=Qt::DisplayRole ) const {
		if ( !index.isValid() )
			return QVariant();
		if ( role == Qt::DecorationRole && index.row() < filters->count() )
			return QVariant( QImage( QString(":/images/icons/%1.png").arg( filters->at( index.row() ).icon ) ) );
		if ( role == Qt::DisplayRole && index.row() < filters->count() ) {
			return QVariant( filters->at( index.row() ).name );
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
		//types << MIMETYPEEFFECT;
		return types;
	}
	
	QMimeData* mimeData(const QModelIndexList &indexes) const {
		QMimeData *mimeData = new QMimeData();
		QByteArray encodedData;
		foreach ( const QModelIndex &index, indexes ) {
			if ( index.isValid() && index.row() < filters->count() ) {
				encodedData.append( filters->at( index.row() ).identifier );
			}
		}
		mimeData->setData( MIMETYPEEFFECT, encodedData );
		return mimeData;
	}
	
private:
	QList<FilterEntry> *filters;
};



class EffectListView : public QListView
{
public:
	EffectListView( QWidget *parent = 0 ) : QListView( parent ) {}
	
protected:
	virtual void startDrag ( Qt::DropActions supportedActions ) {
		QDrag *drag = new QDrag( this );
		const QModelIndexList indexes = selectedIndexes();
		drag->setMimeData( model()->mimeData( indexes ) );
		QPixmap pix = QPixmap::fromImage( model()->data( indexes.first(), Qt::DecorationRole ).value<QImage>() );
		if ( !pix.isNull() ) {
			QString s = model()->data( indexes.first(), Qt::DisplayRole ).toString();
			QPainter p;
			p.begin( &pix );
			QRect r = p.viewport();
			p.setPen( QColor(0,0,0,0) );
			p.setBrush( QColor(0,0,0,128) );
			p.drawRect( r );
			p.setPen( "white" );
			p.drawText( r, Qt::AlignCenter | Qt::TextWordWrap, s );
			p.end();
		}
		drag->setPixmap( pix );
		drag->exec( supportedActions );
	}

};

#endif // EFFECTLISTVIEW_H
