#ifndef GLCOMPOSITION_H
#define GLCOMPOSITION_H

#define GL_GLEXT_PROTOTYPES

#include "movit/effect.h"

#include <QGLShaderProgram>

#include "engine/frame.h"

using namespace movit;



class GLComposition : public QObject
{
	Q_OBJECT
public:
	GLComposition() {
		valid = false;
		posInTrack = 0;
		len = 0;
		invert = false;
	}
	virtual ~GLComposition() {}
	virtual bool process( Effect *e, Frame *src, Frame *dst, Profile *p ) = 0;
	virtual Effect* getMovitEffect() = 0;
	
	bool isValid() { return valid; }
	void setInvert( bool i ) { invert = i; }
	QString getDescriptor() { return compositionName; }

public slots:
	void setPosition( double p ) { posInTrack = p; }
	double position() { return posInTrack; }
	void setLength( double l ) { len = l; }
	double length() { return len; }

protected:
	bool valid;
	double posInTrack, len;
	QString compositionName;
	bool invert;

signals:
	void updateFrame();
};

#endif //GLCOMPOSITION_H
