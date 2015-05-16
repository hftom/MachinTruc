#ifndef FXGRAPHICSVIEW_H
#define FXGRAPHICSVIEW_H

#include <QGraphicsView>
#include <QResizeEvent>
#include <QDebug>



class FxGraphicsView : public QGraphicsView
{
	Q_OBJECT
public:
	FxGraphicsView( QWidget *parent ) : QGraphicsView( parent ) {}
	
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

#endif //FXGRAPHICSVIEW_H
