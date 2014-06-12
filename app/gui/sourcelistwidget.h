#ifndef SOURCELISTWIDGET_H
#define SOURCELISTWIDGET_H

#include <QListWidget>
#include <QMouseEvent>
#include <QDrag>

#include "mimetypes.h"



class SourceListWidget : public QListWidget
{
public:
	SourceListWidget( QWidget *parent ) : QListWidget( parent ) {}
	
protected:
	void mousePressEvent( QMouseEvent *event ) {
		if ( event->button() == Qt::LeftButton ) {
			dragStartPosition = event->pos();
		}
		QListWidget::mousePressEvent( event );
	}
	
	void mouseMoveEvent( QMouseEvent *event ) {
		QListWidgetItem *it = itemAt( dragStartPosition );
		if ( !(event->buttons() & Qt::LeftButton) || !it )
			return;
		if ( (event->pos() - dragStartPosition).manhattanLength() < 20 )
			return;
		QDrag *drag = new QDrag( this );
		QMimeData *mimeData = new QMimeData();
		QByteArray encodedData;
		encodedData.append( QString::number( row( it ) ) + " " );
		encodedData.append( "source" );
		mimeData->setData( MIMETYPESOURCE, encodedData );
		drag->setMimeData(mimeData);
		//drag->setPixmap( QPixmap(":/images/icons/sound.png") );
		drag->exec();
	}
	
private:
	QPoint dragStartPosition;
};

#endif // SOURCELISTWIDGET_H