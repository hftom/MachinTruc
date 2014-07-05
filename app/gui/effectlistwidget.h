#ifndef EFFECTLISTWIDGET_H
#define EFFECTLISTWIDGET_H

#include <QListWidget>
#include <QMouseEvent>
#include <QDrag>

#include "mimetypes.h"



class EffectListWidget : public QListWidget
{
public:
	EffectListWidget( QWidget *parent ) : QListWidget( parent ) {}
	
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
		encodedData.append( it->data( 100 ).toString() );
		mimeData->setData( MIMETYPEEFFECT, encodedData );
		drag->setMimeData( mimeData );
		//drag->setPixmap( QPixmap(":/images/icons/sound.png") );
		drag->exec();
	}
	
private:
	QPoint dragStartPosition;
};

#endif // EFFECTLISTWIDGET_H