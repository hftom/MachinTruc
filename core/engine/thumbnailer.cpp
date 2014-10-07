#include <movit/resample_effect.h>
#include "engine/movitchain.h"
#include "engine/filtercollection.h"

#include <QGLFramebufferObject>

#include "input/input_ff.h"
#include "input/input_image.h"
#include "engine/source.h"

#include "thumbnailer.h"

#define ICONSIZEWIDTH 80.
#define ICONSIZEHEIGHT 45.
#define THUMBHEIGHT 60.



Thumbnailer::Thumbnailer()
	: running( false ),
	glContext( NULL )
{
}


void Thumbnailer::setSharedContext( QGLWidget *sharedContext )
{
	glContext = sharedContext;
}



bool Thumbnailer::pushRequest( ThumbRequest req )
{
	if ( !glContext )
		return false;

	requestMutex.lock();
	requestList.append( req );
	requestMutex.unlock();
	
	if ( !isRunning() ) {
#if QT_VERSION >= 0x050000
		glContext->context()->moveToThread( this );
#endif
		running = true;
		start();
	}
	
	return true;
}



void Thumbnailer::gotResult()
{
	resultMutex.lock();
	if ( !resultList.count() ) {
		resultMutex.unlock();
		return;
	}
	ThumbRequest reqResult = resultList.takeFirst();
	resultMutex.unlock();
	
	emit thumbReady( reqResult.result );
}



void Thumbnailer::run()
{
	glContext->makeCurrent();
	
	while ( running ) {
		requestMutex.lock();
		if ( !requestList.count() ) {
			requestMutex.unlock();
			break;
		}
		ThumbRequest request = requestList.takeFirst();
		requestMutex.unlock();
		
		if ( request.typeOfRequest == ThumbRequest::PROBE ) {
			probe( request );
		}
		else {
			request.result.isValid = false;
		}
		
		resultMutex.lock();
		resultList.append( request );
		resultMutex.unlock();
		emit resultReady();
	}
	
	glContext->doneCurrent();
#if QT_VERSION >= 0x050000
	glContext->context()->moveToThread( qApp->thread() );
#endif
}


void Thumbnailer::probe( ThumbRequest &request )
{
	InputBase *input = new InputImage();
	bool probed = false;
	if ( input->probe( request.filePath, &request.result.profile ) )
		probed = true;
	else {
		delete input;
		input = new InputFF();
		if ( input->probe( request.filePath, &request.result.profile ) )
			probed = true;
	}
	if ( probed ) {
		if ( request.result.profile.hasVideo() )
			request.result.thumb = getSourceThumb( input->getVideoFrame() );
		else {
			request.result.thumb = QImage( ICONSIZEWIDTH + 4, ICONSIZEHEIGHT + 4, QImage::Format_ARGB32 );
			request.result.thumb.fill( QColor(0,0,0,0) );
			QPainter p(&request.result.thumb);
			p.drawImage( 2, 2, QImage(":/images/icons/sound.png") );
		}			
		request.result.inputType = input->getType();
		request.result.path = request.filePath;
		request.result.thumbPTS = request.thumbPTS;
		request.result.isValid = true;
	}
	else {
		request.result.path = request.filePath;
		request.result.isValid = false;
	}
	delete input;
}



QImage Thumbnailer::getSourceThumb( Frame *f )
{
	if ( !f )
		return QImage();

	MovitChain *movitChain = new MovitChain();
	double ar = f->profile.getVideoSAR() * f->profile.getVideoWidth() / f->profile.getVideoHeight();
	movitChain->chain = new EffectChain( ar, 1.0 );
	MovitInput *in = new MovitInput();
	MovitBranch *branch = new MovitBranch( in );
	movitChain->branches.append( branch );
	movitChain->chain->add_input( in->getMovitInput( f ) );
	branch->input->process( f );
			
	// deinterlace
	if ( f->profile.getVideoInterlaced() ) {
		Effect *e = new MyDeinterlaceEffect();
		movitChain->chain->add_effect( e );
	}
			
	int iw, ih;
	if ( ar >= ICONSIZEWIDTH / ICONSIZEHEIGHT ) {
		iw = ICONSIZEWIDTH;
		ih = iw / ar;
	}
	else {
		ih = ICONSIZEHEIGHT;
		iw = ih * ar;
	}
	// resize
	Effect *e = new ResampleEffect();
	if ( e->set_int( "width", iw ) && e->set_int( "height", ih ) )
		movitChain->chain->add_effect( e );
	else
		delete e;
	
	movitChain->chain->set_dither_bits( 8 );
	ImageFormat output_format;
	output_format.color_space = COLORSPACE_sRGB;
	output_format.gamma_curve = GAMMA_sRGB;
	movitChain->chain->add_output( output_format, OUTPUT_ALPHA_FORMAT_POSTMULTIPLIED );
	movitChain->chain->finalize();
	
	// render
	QGLFramebufferObject *fbo = new QGLFramebufferObject( iw, ih );
	movitChain->chain->render_to_fbo( fbo->handle(), iw, ih );
	
	uint8_t data[iw*ih*4];
	fbo->bind();
	glReadPixels(0, 0, iw, ih, GL_BGRA, GL_UNSIGNED_BYTE, data);
	fbo->release();
	
	QImage img( ICONSIZEWIDTH + 4, ICONSIZEHEIGHT + 4, QImage::Format_ARGB32 );
	img.fill( QColor(0,0,0,0) );
	QPainter p(&img);
	p.drawImage( (ICONSIZEWIDTH - iw) / 2 + 2, (ICONSIZEHEIGHT - ih) / 2 + 2, QImage( data, iw, ih, QImage::Format_ARGB32 ).mirrored() );

	f->release();
	delete movitChain;
	delete fbo;
	
	return img;
}
