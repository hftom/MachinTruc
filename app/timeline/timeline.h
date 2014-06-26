#ifndef TIMELINE_H
#define TIMELINE_H

#include <QGraphicsScene>

#include "engine/scene.h"
#include "clipviewitem.h"
#include "cursorviewitem.h"
#include "trackviewitem.h"



class TopWindow;



class DroppedCut
{
public:
	DroppedCut() {
		reset();
	}
	void destroy() {
		delete clipItem;
		delete clip;
		reset();
	}
	void reset() {
		clipItem = NULL;
		clip = NULL;
		shown = false;
	}
	
	ClipViewItem *clipItem;
	Clip* clip;
	bool shown;
	double enterPos;
};



class Timeline : public QGraphicsScene
{
	Q_OBJECT
public:
	Timeline( TopWindow *parent );
	~Timeline();
	
	void clipItemCanMove( ClipViewItem *clip, QPointF mouse, double clipStartPos, QPointF clipStartMouse );
	void clipItemMoved( ClipViewItem *clip, QPointF clipMouseStart );
	void clipItemCanResize( ClipViewItem *clip, int way, QPointF mouse, double clipStartPos, double clipStartLen, QPointF clipStartMouse );
	void clipItemResized( ClipViewItem *clip, int way );
	void addFilter( ClipViewItem *clip, QString fx );
	
	void trackPressed( QPointF p );
	void itemSelected( AbstractViewItem *it );
	
public slots:
	void viewSizeChanged( const QSize &size );
	void setCursorPos( double pts );
	void addTrack( int index );
	
	void setScene( Scene *s );
	void deleteClip();
	
	void filterDeleted( Clip *c, Filter *f );
	
protected:
	/*void mousePressEvent ( QGraphicsSceneMouseEvent *e );
	void mouseMoveEvent( QGraphicsSceneMouseEvent *e );
	void mouseReleaseEvent( QGraphicsSceneMouseEvent *e );*/
	void wheelEvent( QGraphicsSceneWheelEvent *e );
	
	void dragEnterEvent( QGraphicsSceneDragDropEvent *event );
	void dragLeaveEvent( QGraphicsSceneDragDropEvent *event );
	void dragMoveEvent( QGraphicsSceneDragDropEvent *event );
	void dropEvent( QGraphicsSceneDragDropEvent *event );
	
private slots:
	void updateLength();
	
private:
	int getTrack( const QPointF &p );
	void snapMove( ClipViewItem *item, double &pos, double mouseX, double itemScenePos );
	void snapResize( ClipViewItem *item, int way, double &len, double mouseX, double itemScenePos );
	
	CursorViewItem *cursor;
	double zoom;
	int viewWidth;
	
	AbstractViewItem *moveItem;
	double moveItemPos, moveItemScenePos;
	int moveItemTrack;
	
	QList<TrackViewItem*> tracks;
	
	AbstractViewItem *selectedItem;
	
	Scene *scene;
	TopWindow *topParent;
	
	DroppedCut droppedCut;
	
signals:
	void ensureVisible( const QGraphicsItem* );
	void centerOn( const QGraphicsItem* );
	void seekTo( double );
	void updateFrame();
	void clipSelected( Clip* );
};

#endif //TIMELINE_H