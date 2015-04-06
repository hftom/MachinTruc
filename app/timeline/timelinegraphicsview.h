#ifndef TIMELINEGRAPHICSVIEW_H
#define TIMELINEGRAPHICSVIEW_H

#include <QGraphicsView>
#include <QResizeEvent>
#include <QDebug>



class TimelineGraphicsView : public QGraphicsView
{
	Q_OBJECT
public:
	TimelineGraphicsView( QWidget *parent ) : QGraphicsView( parent ) {
		setMouseTracking( true );
	}
	
public slots:
	void showVerticalScrollBar( bool b ) {
		if ( b )
			setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOn );
		else
			setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
	}

protected:
	void resizeEvent( QResizeEvent *e ) {
		QGraphicsView::resizeEvent( e );
		emit sizeChanged( e->size() ); 
	}
	
	virtual void mouseMoveEvent( QMouseEvent * event ) {
		emit mouseMoveTracking( mapToScene( event->pos() ) );
		QGraphicsView::mouseMoveEvent( event );
	}
	
signals:
	void sizeChanged( const QSize& );
	void mouseMoveTracking( QPointF pos );
};

#endif //TIMELINEGRAPHICSVIEW_H
