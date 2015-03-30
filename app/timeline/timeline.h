#ifndef TIMELINE_H
#define TIMELINE_H

#include <QGraphicsScene>

#include "engine/scene.h"
#include "clipviewitem.h"
#include "cursorviewitem.h"
#include "trackviewitem.h"
#include "transitionviewitem.h"
#include "clipeffectviewitem.h"



class TopWindow;



class DroppedCut
{
public:
	DroppedCut() : enterPos(0) {
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
	
	void clipItemCanMove( ClipViewItem *clip, QPointF mouse, double clipStartPos, QPointF clipStartMouse, bool unsnap, bool multiMove );
	void clipItemMoved( ClipViewItem *clip, QPointF clipMouseStart, bool multiMove );
	void clipItemCanResize( ClipViewItem *clip, int way, QPointF mouse, double clipStartPos, double clipStartLen, QPointF clipStartMouse, bool unsnap );
	void clipItemResized( ClipViewItem *clip, int way );
	
	void effectCanMove( QPointF mouse, double clipStartPos, QPointF clipStartMouse, bool unsnap );
	void effectMoved( QPointF clipMouseStart );
	void effectCanResize( int way, QPointF mouse, double clipStartPos, double clipStartLen, QPointF clipStartMouse, bool unsnap );
	void effectResized( int way );
	
	void transitionSelected( TransitionViewItem *it );
	void clipDoubleClicked();
	void clipRightClick( ClipViewItem *cv );
	
	void trackPressed( QPointF p );
	void trackPressedRightBtn( TrackViewItem *t, QPoint p );
	void itemSelected( AbstractViewItem *it );
	
	void trackRemoved( int index );
	void trackAdded( int index );
	
	void thumbResultReady( ThumbRequest result );
	
public slots:
	void viewSizeChanged( const QSize &size );
	void setCursorPos( double pts );
	void addTrack( int index );
	
	void setScene( Scene *s );
	void addFilter( ClipViewItem *clip, QString fx, int index = -1 );
	void deleteClip();
	void splitCurrentClip();
	
	void filterDeleted( Clip *c, QSharedPointer<Filter> f );
	
	void showEffect( bool isVideo, int index );
	
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
	void snapMove( AbstractViewItem *item, double &pos, double mouseX, double itemScenePos, bool limit = false );
	void snapResize( AbstractViewItem *item, int way, double &len, double mouseX, double itemScenePos );

	void updateTransitions( ClipViewItem *clip, bool remove );
	
	void clipThumbRequest( ClipViewItem *it, bool start );
	
	CursorViewItem *cursor;
	double zoom;
	int viewWidth;
	
	QList<TrackViewItem*> tracks;
	
	AbstractViewItem *selectedItem;
	ClipEffectViewItem *effectItem;
	
	Scene *scene;
	TopWindow *topParent;
	
	DroppedCut droppedCut;
	
signals:
	void ensureVisible( const QGraphicsItem* );
	void centerOn( const QGraphicsItem* );
	void seekTo( double );
	void updateFrame();
	void clipSelected( ClipViewItem* );
	void showTransition();
	void showEffects();
	void clipAddedToTimeline( Profile );
	void trackRequest( bool, int );
};

#endif //TIMELINE_H
