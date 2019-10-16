#include "vfx/glpanzoom.h"



GLPanZoom::GLPanZoom( QString id, QString name ) : GLSize( id, name ), pw(0), ph(0), sw(0), sh(0)
{
	sizePercent->hidden = true;
	xOffset->hidden = true;
	yOffset->hidden = true;
	rotateAngle->hidden = true;
	//softBorder->hidden = true;
	
	animType = rand() % 2;
	xDirection = rand() % 2 ? -1.0 : 1.0;
	yDirection = rand() % 2 ? -1.0 : 1.0;
}



void GLPanZoom::preProcess(double pts, Frame *src, Profile *p )
{
	if (pw == p->getVideoWidth() && ph == p->getVideoHeight() && sw == src->glWidth && sh == src->glHeight) {
		return;
	}
	
	pw = p->getVideoWidth();
	ph = p->getVideoHeight();
	sw = src->glWidth;
	sh = src->glHeight;
	
	double fullscreenZoom = (sw / sh < pw / ph) ? ph / sh : pw / sw;
	// no border zoom
	double zoom = 1.0;
	if (sw * fullscreenZoom < pw) {
		zoom = pw / (sw * fullscreenZoom);
	}
	if (sh * fullscreenZoom * zoom < ph) {
		zoom = ph / (sh * fullscreenZoom);
	}
	double percent = zoom * 100.0;
	
	sizePercent->graph.keys.clear();
	xOffset->graph.keys.clear();
	yOffset->graph.keys.clear();
	
	if (animType == 0) {
		if (xDirection > 0) {
			sizePercent->graph.keys.append( AnimationKey( AnimationKey::LINEAR, 0, percent / sizePercent->max.toDouble() ) );
			sizePercent->graph.keys.append( AnimationKey( AnimationKey::LINEAR, 1, (percent + 10.0) / sizePercent->max.toDouble() ) );
		}
		else {
			sizePercent->graph.keys.append( AnimationKey( AnimationKey::LINEAR, 0, (percent + 10.0) / sizePercent->max.toDouble() ) );
			sizePercent->graph.keys.append( AnimationKey( AnimationKey::LINEAR, 1, percent / sizePercent->max.toDouble() ) );
		}
	}
	else {
		sizePercent->value = percent + 10.0;
		
		double pos = 5.0 / zoom;
		xOffset->graph.keys.append( AnimationKey( AnimationKey::LINEAR, 0, xOffset->getNormalizedKeyValue(xDirection * pos) ) );
		xOffset->graph.keys.append( AnimationKey( AnimationKey::LINEAR, 1, xOffset->getNormalizedKeyValue(-xDirection * pos) ) );
		yOffset->graph.keys.append( AnimationKey( AnimationKey::LINEAR, 0, yOffset->getNormalizedKeyValue(yDirection * pos) ) );
		yOffset->graph.keys.append( AnimationKey( AnimationKey::LINEAR, 1, yOffset->getNormalizedKeyValue(-yDirection * pos) ) );
	}
}
