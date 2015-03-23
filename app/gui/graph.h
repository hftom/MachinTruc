#ifndef GRAPH_H
#define GRAPH_H

#include <QGraphicsScene>
#include <QPainter>

#include "engine/clip.h"
#include "engine/thumbnailer.h"



class GraphEffectItem;



class Graph : public QGraphicsScene
{
	Q_OBJECT
public:
	Graph( bool audio = false );
	
	void setCurrentClip( Clip *c );
	Clip *getClip() { return currentClip; }
	
public slots:
	void viewSizeChanged( const QSize &size );
	void itemSelected( GraphEffectItem *it );
	void effectRightClick( GraphEffectItem *it );

private slots:
	void updateLength();
	void rebuildGraph();
	
private:
	bool isAudio;
	int viewHeight;
	Clip *currentClip;
	GraphEffectItem *selectedItem;
	
signals:
	void filterSelected( Clip*, int index );
	void filterDeleted( Clip*, QSharedPointer<Filter> );
};

#endif // GRAPH_H