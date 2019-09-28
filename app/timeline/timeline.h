#ifndef TIMELINE_H
#define TIMELINE_H

#include <QGraphicsScene>
#include <QPropertyAnimation>
#include <QUndoStack>

#include "engine/scene.h"
#include "clipviewitem.h"
#include "cursorviewitem.h"
#include "rulerdock.h"
#include "rulerviewitem.h"
#include "trackviewitem.h"
#include "transitionviewitem.h"
#include "clipeffectviewitem.h"
#include "selectwindowitem.h"

#include "gui/clipboard.h"



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
	Q_PROPERTY( qreal animZoom READ getCurrentZoom WRITE setCurrentZoom )
public:
	Timeline( TopWindow *parent, QUndoStack *stack );
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
	
	void undockRuler();
	void dockRuler();
	
	void trackPressed( QPointF p );
	void trackPressedRightBtn( TrackViewItem *t, QPoint p );
	void trackSelectWindow( QPointF p );
	void trackSelectWindowMove( QPointF p );
	void trackSelectWindowRelease(bool extend);
	void itemSelected( AbstractViewItem *it, bool extend = false, bool moreToCome = false );
	
	void playheadMoved( double p );
	
	void trackRemoved( int index );
	void trackAdded( int index );
	void addTrack( int index, bool noUndo = false );
	
	void thumbResultReady( ThumbRequest result );
	
	void zoomInOut( bool in );
	void selectAll();
	void editCopy(ClipBoard *clipboard);
	void editCut(ClipBoard *clipboard);
	void editPaste(ClipBoard *clipboard);
	
	void commandAddClip(QList<Clip*> clips, QList<int> ltracks, QList<Transition*> tails);
	void commandRemoveClip(QList<Clip*> clips, QList<int> ltracks);
	void commandMoveClip(Clip *clip, bool multi, int oldTrack, int newTrack, double pos, Transition *trans, Transition *tail);
	void commandResizeClip(Clip *clip, bool resizeStart, int track, double position, double length, Transition *trans);
	void commandClipSpeed(Clip *c, int track, double speed, double length, Transition *tail);
	void commandSplitClip(Clip *c, Clip *c1, Clip *c2, int track, Transition *trans, Transition *tail, bool redo);
	void commandEffectAddRemove(QList<Clip*> clips, QList<int> ltracks, QList< QSharedPointer<Filter> > filters, bool isVideo, QList<int> indexes, bool remove);
	void commandTrackAddRemove(int index, bool remove, bool noparent = false);
	void commandEffectMove(Clip *c, double newPos, bool isVideo, int index);
	void commandEffectResize(Clip *c, bool resizeStart, double offset, double position, double length, bool video, int effectIndex);
	void commandEffectReorder(Clip *c, int track, int oldIndex, int newIndex, bool isVideo);
	void commandEffectParam(QSharedPointer<Filter> filter, Parameter *param, QVariant value);
	void commandTransitionChanged(Clip *clip, QSharedPointer<Filter> oldFilter, QString newFilter, bool isVideo, bool undo);
	
public slots:
	void nextEdge();
	void previousEdge();

	void viewMouseMove( QPointF pos );
	void viewMouseLeave();
	void viewSizeChanged( const QSize &size );
	void setCursorPos( double pts, bool isPlaying );
	
	void setScene( Scene *s );
	void addFilter( ClipViewItem *clip, QString fx, int index = -1 );
	void splitCurrentClip();
	
	void filterDeleted( Clip *c, QSharedPointer<Filter> f );
	void filterReordered( Clip *c, bool video, int index, int newIndex );
	void paramUndoCommand(QSharedPointer<Filter> f, Parameter *p, QVariant oldValue, QVariant newValue);
	
	void transitionChanged(Clip *clip, QString filterName, bool isVideo);
	
	void showEffect( bool isVideo, int index );
	
	void addSelectionToTimeline();
	
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
	void slotUpdateAfterEdit();
	
private:
	void updateAfterEdit(bool doFrame, bool doLength);
	void updateLength();
	ClipViewItem* getSelectedClip();
	void updateStabilize(Clip *clip, Filter *f, bool stop);
	int getTrack( const QPointF &p );
	ClipViewItem* getClipViewItem(Clip *clip, int track);
	void snapMove( AbstractViewItem *item, double &pos, double mouseX, double itemScenePos, bool limit = false );
	void snapResize( AbstractViewItem *item, int way, double &len, double mouseX, double itemScenePos );

	void updateTransitions( ClipViewItem *clip, bool remove );
	
	void clipThumbRequest( ClipViewItem *it, bool start );
	
	qreal getCurrentZoom();
	void setCurrentZoom( qreal z );
	
	RulerDock *rulerDock;
	CursorViewItem *cursor;
	RulerViewItem *ruler;
	double zoom, currentZoom;
	int viewWidth;
	
	QList<TrackViewItem*> tracks;
	
	QList<AbstractViewItem*> selectedItems;
	ClipEffectViewItem *effectItem;
	
	Scene *scene;
	TopWindow *topParent;
	
	DroppedCut droppedCut;
	SelectWindowItem *selectWindowItem;
	
	QPropertyAnimation *zoomAnim;
	
	QPointF mouseScenePosition;
	
	bool forceEnsureVisible;
	
	QUndoStack *undoStack;
	
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
