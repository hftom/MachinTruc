#include <QApplication>

#include <movit/init.h>
#include <movit/resample_effect.h>
#include "engine/movitchain.h"
#include "engine/filtercollection.h"
#include "vfx/movitflip.h"
#include "vfx/movitbackground.h"

#include <QGLShaderProgram>
#include <QGLFramebufferObject>
#include <QCryptographicHash>

#include "input/input_ff.h"
#include "input/input_image.h"
#include "input/input_blank.h"
#include "engine/source.h"
#include "util.h"
#include "thumbnailer.h"

#define THUMB_DIR "thumb"
#define THUMB_EXTENSION ".png"



Thumbnailer::Thumbnailer()
	: running( false ),
	glContext( NULL )
{
}



Thumbnailer::~Thumbnailer()
{
	running = false;
	wait();
}



bool Thumbnailer::cdThumbDir( QDir &dir )
{
	dir = QDir::home();
	if ( !dir.cd( MACHINTRUC_DIR ) ) {
		if ( !dir.mkdir( MACHINTRUC_DIR ) ) {
			qDebug() << "Can't create" << MACHINTRUC_DIR << "directory.";
			return false;
		}
		if ( !dir.cd( MACHINTRUC_DIR ) )
			return false;
	}
	if ( !dir.cd( THUMB_DIR ) ) {
		if ( !dir.mkdir( THUMB_DIR ) ) {
			qDebug() << "Can't create" << THUMB_DIR << "directory.";
			return false;
		}
		if ( !dir.cd( THUMB_DIR ) )
			return false;
	}

	return true;
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
	if ( req.typeOfRequest == ThumbRequest::SHADER )
		requestList.prepend( req );
	else
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

	emit thumbReady( reqResult );
}



void Thumbnailer::run()
{
	glContext->makeCurrent();
	
	while ( running ) {
		requestMutex.lock();
		if ( !requestList.count() ) {
			requestMutex.unlock();
			usleep( 100000 );
		}
		else {
			ThumbRequest request = requestList.takeFirst();
			requestMutex.unlock();
		
			if ( request.typeOfRequest == ThumbRequest::PROBE ) {
				probe( request );
			}
			else if ( request.typeOfRequest == ThumbRequest::SHADER ) {
				compileShader( request );
			}
			else {
				makeThumb( request );
			}
		
			resultMutex.lock();
			resultList.append( request );
			resultMutex.unlock();
			emit resultReady();
		}
	}
	
	glContext->doneCurrent();
#if QT_VERSION >= 0x050000
	glContext->context()->moveToThread( qApp->thread() );
#endif
}



void Thumbnailer::probe( ThumbRequest &request )
{
	QDir dir;
	if ( request.inputType != InputBase::UNDEF && cdThumbDir( dir ) ) {
		QString s = QCryptographicHash::hash( request.filePath.toUtf8(), QCryptographicHash::Sha256 ).toHex();
		if ( QFile::exists( dir.filePath( s + THUMB_EXTENSION ) ) ) {
			request.thumb = QImage( dir.filePath( s + THUMB_EXTENSION ) );
			if ( !request.thumb.isNull() ) {
				return;
			}
		}
	}

	InputBase *input = new InputBlank();
	bool probed = false;
	if ( input->probe( request.filePath, &request.profile ) )
		probed = true;
	else {
		delete input;
		input = new InputImage();
		if ( input->probe( request.filePath, &request.profile ) )
			probed = true;
	}
	if ( !probed ) {
		delete input;
		input = new InputFF();
		if ( input->probe( request.filePath, &request.profile ) )
			probed = true;
	}
	if ( probed ) {
		if ( request.profile.hasVideo() ) {
			input->seekTo( request.profile.getStreamStartTime() + request.profile.getStreamDuration() / 10.0 );
			request.thumb = getSourceThumb( input->getVideoFrame(), true );
			if ( !request.thumb.isNull() ) {
				if ( cdThumbDir( dir ) ) {
					QString s = QCryptographicHash::hash( request.filePath.toUtf8(), QCryptographicHash::Sha256 ).toHex();
					request.thumb.save( dir.filePath( s + THUMB_EXTENSION ) );
				}
			}
		}
		else {
			request.thumb = QImage( ICONSIZEWIDTH + 4, ICONSIZEHEIGHT + 4, QImage::Format_ARGB32 );
			request.thumb.fill( QColor(0,0,0,0) );
			QPainter p(&request.thumb);
			p.drawImage( 2, 2, QImage(":/images/icons/sound.png") );
		}			
		request.inputType = input->getType();
	}

	delete input;
}



void Thumbnailer::makeThumb( ThumbRequest &request )
{
	int nf = (request.thumbPTS - request.profile.getStreamStartTime()) / request.profile.getVideoFrameDuration();
	QString filename = QCryptographicHash::hash( request.filePath.toUtf8(), QCryptographicHash::Sha256 ).toHex();
	filename += "." + QString::number( nf ) + THUMB_EXTENSION;
	QDir dir;
	if ( cdThumbDir( dir ) ) {
		if ( QFile::exists( dir.filePath( filename ) ) ) {
			request.thumb = QImage( dir.filePath( filename ) );
			if ( !request.thumb.isNull() ) {
				return;
			}
		}
	}

	InputBase *input;
	if ( request.inputType == InputBase::GLSL )
		input = new InputBlank();
	else if ( request.inputType == InputBase::IMAGE )
		input = new InputImage();
	else 
		input = new InputFF();
	
	input->setProfile( request.profile, request.profile );
	input->open( request.filePath );
	
	if ( request.profile.hasVideo() ) {
		input->seekTo( request.thumbPTS );
		Frame *f = input->getVideoFrame();
		if ( !f ) {
			input->openSeekPlay( request.filePath, request.thumbPTS - MICROSECOND );
			for ( int i = 0; i < request.profile.getVideoFrameRate() + 2; ++i )
				f = input->getVideoFrame();
		}
		request.thumb = getSourceThumb( f, false );
		if ( !request.thumb.isNull() ) {
			if ( cdThumbDir( dir ) ) {
				request.thumb.save( dir.filePath( filename ) );
			}
		}
	}
	else {
		request.thumb = QImage( ICONSIZEWIDTH, ICONSIZEHEIGHT, QImage::Format_ARGB32 );
		request.thumb.fill( QColor(0,0,0,0) );
		QPainter p(&request.thumb);
		p.drawImage( 0, 0, QImage(":/images/icons/sound.png") );
	}			
	
	delete input;
}



void Thumbnailer::compileShader( ThumbRequest &request )
{
	int faulty;
	QList<Parameter> list = Parameter::parseShaderParams( request.filePath, faulty );
	if ( faulty != -1 ) {
		request.filePath = QString( "nok Error at line %1" ).arg( faulty );
		return;
	}
	for ( int i = 0; i < list.count(); ++i ) {
		Parameter p = list.at(i);
		if ( p.type == Parameter::PDOUBLE )
			request.filePath.prepend( QString( "uniform float PREFIX(%1);\n" ).arg( p.id ) );
		else if ( p.type == Parameter::PRGBCOLOR )
			request.filePath.prepend( QString( "uniform vec3 PREFIX(%1);\n" ).arg( p.id ) );
		else if ( p.type == Parameter::PRGBACOLOR )
			request.filePath.prepend( QString( "uniform vec4 PREFIX(%1);\n" ).arg( p.id ) );
	}
	
	QString shader = read_version_dependent_file( "header", "frag" ).c_str();
	shader += "uniform sampler2D tex;\n";
	shader += "vec4 in0( vec2 tc ) {\n";
	shader += "	return tex2D( tex, tc );\n";
	shader += "}\n";
	shader += "\n";
	shader += "#define INPUT in0\n";
	shader += "#define FUNCNAME eff1\n";
	shader += "\n";
	// declare time and texelSize uniforms
	shader += "uniform float eff1_time;\n";
	shader += "uniform vec2 eff1_texelSize;\n";
	QString s = request.filePath;
	shader += s.replace( QRegExp("PREFIX\\(([^\\)]*)\\)"), "eff1_\\1" );
	shader += "\n";
	shader += "#undef INPUT\n";
	shader += "#undef FUNCNAME\n";
	shader += "#define INPUT eff1\n";
	shader += "#define FUNCNAME eff2\n";
	shader += "\n";
	shader += "uniform float eff2_time;\n";
	shader += "uniform vec2 eff2_texelSize;\n";
	s = request.filePath;
	shader += s.replace( QRegExp("PREFIX\\(([^\\)]*)\\)"), "eff2_\\1" );
	shader += "\n";
	shader += read_file( "footer.frag" ).c_str();
	
	QGLShaderProgram prog;
	bool ok = prog.addShaderFromSourceCode( QGLShader::Fragment, shader )
		&& prog.addShaderFromSourceCode( QGLShader::Vertex, read_version_dependent_file("vs", "vert").c_str());
	if ( ok )
		ok = prog.link();
	if ( ok ) {
		// be strict, treat warning as error
		/*if ( prog.log().contains( "Warning" ) || prog.log().contains( "warning" ) )
			request.filePath = "nok" + prog.log();
		else*/
			request.filePath = "ok";
	}
	else {
		request.filePath = "nok" + prog.log();
	}
}



QImage Thumbnailer::getSourceThumb( Frame *f, bool border )
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

	int iw, ih;
	if ( ar >= ICONSIZEWIDTH / ICONSIZEHEIGHT ) {
		iw = ICONSIZEWIDTH;
		ih = qMax( 1.0, iw / ar );
	}
	else {
		ih = ICONSIZEHEIGHT;
		iw = qMax( 1.0, ih * ar );
	}
	// resize
	Effect *e = new ResampleEffect();
	if ( e->set_int( "width", iw ) && e->set_int( "height", ih ) )
		movitChain->chain->add_effect( e );
	else
		delete e;
	// vertical flip
	movitChain->chain->add_effect( new MyFlipEffect() );
	movitChain->chain->add_effect( new MovitBackgroundEffect() );	
	
	movitChain->chain->set_dither_bits( 8 );
	ImageFormat output_format;
	output_format.color_space = COLORSPACE_sRGB;
	output_format.gamma_curve = GAMMA_REC_709;
	movitChain->chain->add_output( output_format, OUTPUT_ALPHA_FORMAT_POSTMULTIPLIED );
	movitChain->chain->finalize();
	
	// render
	QGLFramebufferObject *fbo = new QGLFramebufferObject( iw, ih );
	movitChain->chain->render_to_fbo( fbo->handle(), iw, ih );
	
	uint8_t data[iw*ih*4];
	fbo->bind();
	glReadPixels(0, 0, iw, ih, GL_BGRA, GL_UNSIGNED_BYTE, data);
	fbo->release();
	
	if ( f->type() == Frame::GLSL ) {
		QImage trans(":/images/icons/transparency.png");
		for ( int i = 0; i < ih; ++i )
			memcpy( data + iw * 4 * i, trans.constScanLine( i ), iw * 4 );
	}
	
	QImage img;
	if ( border ) {
		img = QImage( ICONSIZEWIDTH + 4, ICONSIZEHEIGHT + 4, QImage::Format_ARGB32 );
		img.fill( QColor(0,0,0,0) );
		QPainter p(&img);
		p.drawImage( (ICONSIZEWIDTH - iw) / 2 + 2, (ICONSIZEHEIGHT - ih) / 2 + 2, QImage( data, iw, ih, QImage::Format_ARGB32 ) );
	}
	else {
		img = QImage( ICONSIZEWIDTH, ICONSIZEHEIGHT, QImage::Format_ARGB32 );
		img.fill( QColor(0,0,0) );
		QPainter p(&img);
		p.drawImage( (ICONSIZEWIDTH - iw) / 2, (ICONSIZEHEIGHT - ih) / 2, QImage( data, iw, ih, QImage::Format_ARGB32 ) );
	}

	delete f;
	delete movitChain;
	delete fbo;
	
	return img;
}
