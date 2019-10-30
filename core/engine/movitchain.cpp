#include <movit/ycbcr_input.h>
#include <movit/flat_input.h>

#include "engine/movitchain.h"

#define OFFSET_BUFFER(i) ((uint8_t*)NULL + (i))



MovitInput::MovitInput()
	: input( NULL ),
	mmi( -1 )
{
}



MovitInput::~MovitInput()
{
}



bool MovitInput::setBuffer( PBO *p, Frame *src, int size )
{
	glBindBuffer( GL_PIXEL_UNPACK_BUFFER_ARB, p->pbo() );
	void *mem = glMapBuffer( GL_PIXEL_UNPACK_BUFFER_ARB, GL_WRITE_ONLY );
	if ( !mem ) {
		p->setFree( true );
		return false;
	}
	memcpy( mem, src->data(), size );
	glUnmapBuffer( GL_PIXEL_UNPACK_BUFFER_ARB );
	glBindBuffer( GL_PIXEL_UNPACK_BUFFER_ARB, 0 );
	src->setPBO( p );
	return true;
}



void MovitInput::setPixelData8(Frame *src, int size, int stride[], GLResource *gl )
{
	YCbCrInput *ycbcr = (YCbCrInput*)input;
	PBO *p = NULL;
	if ( gl )
		p = gl->getPBO( size );
	if ( p && setBuffer( p, src, size ) ) {
		ycbcr->set_pixel_data( 0, OFFSET_BUFFER( stride[0] ), p->pbo() );
		ycbcr->set_pixel_data( 1, OFFSET_BUFFER( stride[1] ), p->pbo());
		ycbcr->set_pixel_data( 2, OFFSET_BUFFER( stride[2] ), p->pbo() );
	}
	else {
		uint8_t *data = src->data();
		ycbcr->set_pixel_data( 0, &data[stride[0]] );
		ycbcr->set_pixel_data( 1, &data[stride[1]]);
		ycbcr->set_pixel_data( 2, &data[stride[2]] );
	}
}



void MovitInput::setPixelData16(Frame *src, int size, int stride[], GLResource *gl )
{
	YCbCrInput *ycbcr = (YCbCrInput*)input;
	PBO *p = NULL;
	if ( gl )
		p = gl->getPBO( size );
	if ( p && setBuffer( p, src, size ) ) {
		ycbcr->set_pixel_data( 0, reinterpret_cast<const uint16_t*>(OFFSET_BUFFER( stride[0] )), p->pbo() );
		ycbcr->set_pixel_data( 1, reinterpret_cast<const uint16_t*>(OFFSET_BUFFER( stride[1] )), p->pbo());
		ycbcr->set_pixel_data( 2, reinterpret_cast<const uint16_t*>(OFFSET_BUFFER( stride[2] )), p->pbo() );
	}
	else {
		uint8_t *data = src->data();
		ycbcr->set_pixel_data( 0, reinterpret_cast<const uint16_t*>(&data[stride[0]]) );
		ycbcr->set_pixel_data( 1, reinterpret_cast<const uint16_t*>(&data[stride[1]]));
		ycbcr->set_pixel_data( 2, reinterpret_cast<const uint16_t*>(&data[stride[2]]) );
	}
}



bool MovitInput::process( Frame *src, double pts, GLResource *gl)
{
	if ( src->mmiProvider != mmiProvider )
		mmiProvider = src->mmiProvider;
	else if ( mmi != -1 && src->mmi != 0 && src->mmi == mmi )
		return true;
	
	mmi = src->mmi ? src->mmi : ++src->mmi;
	
	int w = src->profile.getVideoWidth();
	int h = src->profile.getVideoHeight();
	int bytes = src->bitDepth > 8 ? 2 : 1;

	switch ( src->type() ) {
		case Frame::YUV420P: {
			int size = w * h * 3 / 2 * bytes;
			int stride[3] = {0, w * h * bytes, (w * h + (w / 2 * h / 2)) * bytes};
			bytes > 1 ? setPixelData16(src, size, stride, gl) : setPixelData8(src, size, stride, gl);
			return true;
		}
		case Frame::YUV422P: {
			int size = w * h * 2 * bytes;
			int stride[3] = {0, w * h *bytes, (w * h + (w / 2 * h)) * bytes};
			bytes > 1 ? setPixelData16(src, size, stride, gl) : setPixelData8(src, size, stride, gl);
			return true;
		}
		case Frame::YUV444P: {
			int size = w * h * 3 *bytes;
			int stride[3] = {0, w * h *bytes, w * h * 2 * bytes};
			bytes > 1 ? setPixelData16(src, size, stride, gl) : setPixelData8(src, size, stride, gl);
			return true;
		}
		case Frame::RGB:
		case Frame::RGBA: {
			uint8_t *data = src->data();
			FlatInput *flat = (FlatInput*)input;
			int size = src->type() == Frame::RGB ? w * h * 3 : w * h * 4;
			PBO *p = NULL;
			if ( gl )
				p = gl->getPBO( size );
			if ( p && setBuffer( p, src, size ) ) {
				flat->set_pixel_data( OFFSET_BUFFER( 0 ), p->pbo() );
			}
			else
				flat->set_pixel_data( data );
			return true;
		}
		case Frame::GLSL:{
			GLSLInput *glsl = (GLSLInput*)input;
			glsl->set_float( "time", pts / MICROSECOND )
			&& glsl->set_float( "iwidth", src->glWidth )
			&& glsl->set_float( "iheight", src->glHeight );
			return true;
		}
	}

	return false;
}



Input* MovitInput::getMovitInput( Frame *src )
{
	ImageFormat input_format;
	YCbCrFormat ycbcr_format;

	ycbcr_format.full_range = src->profile.getVideoColorFullRange();

	switch ( src->profile.getVideoColorSpace() ) {
		case Profile::SPC_UNDEF:
		case Profile::SPC_709:
			ycbcr_format.luma_coefficients = YCBCR_REC_709;
			break;
		case Profile::SPC_601_625:
		case Profile::SPC_601_525:
			ycbcr_format.luma_coefficients = YCBCR_REC_601;
			break;
	}
	
	switch ( src->profile.getVideoGammaCurve() ) {
		case Profile::GAMMA_UNDEF:
		case Profile::GAMMA_709:
			input_format.gamma_curve = GAMMA_REC_709;
			break;
		case Profile::GAMMA_601:
			input_format.gamma_curve = GAMMA_REC_601;
			break;
		case Profile::GAMMA_SRGB:
			input_format.gamma_curve = GAMMA_sRGB;
			break;
	}			

	switch ( src->profile.getVideoColorPrimaries() ) {
		case Profile::PRI_UNDEF:
		case Profile::PRI_709:
			input_format.color_space = COLORSPACE_REC_709;
			break;
		case Profile::PRI_601_625:
			input_format.color_space = COLORSPACE_REC_601_625;
			break;
		case Profile::PRI_601_525:
			input_format.color_space = COLORSPACE_REC_601_525;
			break;
	}
	
	switch ( src->profile.getVideoChromaLocation() ) {
		case Profile::LOC_CENTER:
			ycbcr_format.cb_x_position = ycbcr_format.cr_x_position = 0.5;
			ycbcr_format.cb_y_position = ycbcr_format.cr_y_position = 0.5;
			break;
		case Profile::LOC_TOPLEFT:
			ycbcr_format.cb_x_position = ycbcr_format.cr_x_position = 0;
			ycbcr_format.cb_y_position = ycbcr_format.cr_y_position = 0;
			break;
		case Profile::LOC_LEFT:
		case Profile::LOC_UNDEF:
			ycbcr_format.cb_x_position = ycbcr_format.cr_x_position = 0;
			ycbcr_format.cb_y_position = ycbcr_format.cr_y_position = 0.5;
			break;
	}

	if (src->type() < Frame::RGBA) {
		ycbcr_format.num_levels = pow(2, src->bitDepth);
		GLenum glType = src->bitDepth > 8 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_BYTE;
		
		switch ( src->type() ) {
			case Frame::YUV420P: {
				ycbcr_format.chroma_subsampling_x = ycbcr_format.chroma_subsampling_y = 2;
				break;
			}
			case Frame::YUV422P: {
				ycbcr_format.chroma_subsampling_x = 2;
				ycbcr_format.chroma_subsampling_y = 1;
				break;
			}
			case Frame::YUV444P: {
				ycbcr_format.chroma_subsampling_x = ycbcr_format.chroma_subsampling_y = 1;
				break;
			}
		}
		
		input = new YCbCrInput( input_format, ycbcr_format, src->profile.getVideoWidth(), src->profile.getVideoHeight(), YCBCR_INPUT_PLANAR, glType );
		return input;
	}
	else {
		switch ( src->type() ) {
			case Frame::RGBA: {
				input = new FlatInput( input_format, FORMAT_BGRA_POSTMULTIPLIED_ALPHA, GL_UNSIGNED_BYTE, src->profile.getVideoWidth(), src->profile.getVideoHeight() );
				return input;
			}
			case Frame::RGB: {
				input = new FlatInput( input_format, FORMAT_BGR, GL_UNSIGNED_BYTE, src->profile.getVideoWidth(), src->profile.getVideoHeight() );
				return input;
			}
			case Frame::GLSL: {
				input = new GLSLInput( src->profile.getVideoWidth(), src->profile.getVideoHeight(), src->profile.getVideoCodecName() );
				return input;
			}
		}
	}

	return NULL;
}



QString MovitInput::getDescriptor( Frame *src )
{
	QString s = QString(" %1 %2 %3 %4 %5 %6 %7").arg( src->profile.getVideoWidth() ).arg( src->profile.getVideoHeight() ).arg( src->profile.getVideoColorSpace() )
		.arg( src->profile.getVideoColorPrimaries() ).arg( src->profile.getVideoColorFullRange() ).arg( src->profile.getVideoChromaLocation() )
		.arg( src->profile.getVideoGammaCurve() );
	switch ( src->type() ) {
		case Frame::YUV420P: return "YUV420P" + QString::number(src->bitDepth) + s;
		case Frame::YUV422P: return "YUV422P" + QString::number(src->bitDepth) + s;
		case Frame::YUV444P: return "YUV444P" + QString::number(src->bitDepth) + s;
		case Frame::RGB:
			return QString("FLATINPUT RGB %1 %2").arg( src->profile.getVideoWidth() ).arg( src->profile.getVideoHeight() );
		case Frame::RGBA:
			return QString("FLATINPUT RGBA %1 %2").arg( src->profile.getVideoWidth() ).arg( src->profile.getVideoHeight() );
		case Frame::GLSL:
			return QString("GLSL %1 %2 %3").arg(src->profile.getVideoCodecName()).arg( src->profile.getVideoWidth() ).arg( src->profile.getVideoHeight() );
	}
	return QString();
}



MovitFilter::MovitFilter( const QList<Effect*> &el, GLFilter *f )
	: effects( el ),
	filter( f )
{
}



MovitBranch::MovitBranch( MovitInput *in )
	: input( in ),
	overlay( NULL )
{
}



MovitBranch::~MovitBranch() 
{
	delete input;
	while ( !filters.isEmpty() )
		delete filters.takeFirst();
	if ( overlay )
		delete overlay;
}



MovitChain::MovitChain()
	: chain( NULL )
{
}
	
MovitChain::~MovitChain() 
{
	reset();
}

void MovitChain::reset()
{
	while ( !branches.isEmpty() )
		delete branches.takeFirst();
	if ( chain ) {
		delete chain;
		chain = NULL;
	}
}
