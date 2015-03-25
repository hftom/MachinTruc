#ifndef TIMELINEGRAPHICSVIEW_H
#define TIMELINEGRAPHICSVIEW_H

#include <QGraphicsView>
#include <QResizeEvent>



class TimelineGraphicsView : public QGraphicsView
{
	Q_OBJECT
public:
	TimelineGraphicsView( QWidget *parent ) : QGraphicsView( parent ) {}
	
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
	
signals:
	void sizeChanged( const QSize& );
};

#endif //TIMELINEGRAPHICSVIEW_H
