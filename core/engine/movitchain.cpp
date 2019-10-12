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



bool MovitInput::process( Frame *src, GLResource *gl )
{
	if ( src->mmiProvider != mmiProvider )
		mmiProvider = src->mmiProvider;
	else if ( mmi != -1 && src->mmi != 0 && src->mmi == mmi )
		return true;
	
	mmi = src->mmi ? src->mmi : ++src->mmi;
	
	int w = src->profile.getVideoWidth();
	int h = src->profile.getVideoHeight();

	switch ( src->type() ) {
		case Frame::YUV420P: {
			int size = w * h * 3 / 2;
			int stride[3] = {0, w *h, w * h + (w / 2 * h / 2)};
			setPixelData8(src, size, stride, gl);
			return true;
		}
		case Frame::YUV422P: {
			int size = w * h * 2;
			int stride[3] = {0, w *h, w * h + (w / 2 * h)};
			setPixelData8(src, size, stride, gl);
			return true;
		}
		case Frame::YUV420P12LE:
		case Frame::YUV420P10LE: {
			int size = w * h * 3;
			int stride[3] = {0, w * h * 2, (w * h + (w / 2 * h / 2)) * 2};
			setPixelData16(src, size, stride, gl);
			return true;
		}
		case Frame::YUV422P12LE:
		case Frame::YUV422P10LE: {
			int size = w * h * 4;
			int stride[3] = {0, w * h * 2, (w * h + (w / 2 * h)) * 2};
			setPixelData16(src, size, stride, gl);
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

	switch ( src->type() ) {
		case Frame::YUV420P: {
			ycbcr_format.chroma_subsampling_x = ycbcr_format.chroma_subsampling_y = 2;
			ycbcr_format.num_levels = 256;
			input = new YCbCrInput( input_format, ycbcr_format, src->profile.getVideoWidth(), src->profile.getVideoHeight() );
			return input;
		}
		case Frame::YUV422P: {
			ycbcr_format.chroma_subsampling_x = 2;
			ycbcr_format.chroma_subsampling_y = 1;
			ycbcr_format.num_levels = 256;
			input = new YCbCrInput( input_format, ycbcr_format, src->profile.getVideoWidth(), src->profile.getVideoHeight() );
			return input;
		}
		case Frame::YUV420P10LE: {
			ycbcr_format.chroma_subsampling_x = ycbcr_format.chroma_subsampling_y = 2;
			ycbcr_format.num_levels = 1024;
			input = new YCbCrInput( input_format, ycbcr_format, src->profile.getVideoWidth(), src->profile.getVideoHeight(), YCBCR_INPUT_PLANAR, GL_UNSIGNED_SHORT );
			return input;
		}
		case Frame::YUV422P10LE: {
			ycbcr_format.chroma_subsampling_x = 2;
			ycbcr_format.chroma_subsampling_y = 1;
			ycbcr_format.num_levels = 1024;
			input = new YCbCrInput( input_format, ycbcr_format, src->profile.getVideoWidth(), src->profile.getVideoHeight(), YCBCR_INPUT_PLANAR, GL_UNSIGNED_SHORT );
			return input;
		}
		case Frame::YUV420P12LE: {
			ycbcr_format.chroma_subsampling_x = ycbcr_format.chroma_subsampling_y = 2;
			ycbcr_format.num_levels = 4096;
			input = new YCbCrInput( input_format, ycbcr_format, src->profile.getVideoWidth(), src->profile.getVideoHeight(), YCBCR_INPUT_PLANAR, GL_UNSIGNED_SHORT );
			return input;
		}
		case Frame::YUV422P12LE: {
			ycbcr_format.chroma_subsampling_x = 2;
			ycbcr_format.chroma_subsampling_y = 1;
			ycbcr_format.num_levels = 4096;
			input = new YCbCrInput( input_format, ycbcr_format, src->profile.getVideoWidth(), src->profile.getVideoHeight(), YCBCR_INPUT_PLANAR, GL_UNSIGNED_SHORT );
			return input;
		}
		case Frame::RGBA: {
			input = new FlatInput( input_format, FORMAT_BGRA_POSTMULTIPLIED_ALPHA, GL_UNSIGNED_BYTE, src->profile.getVideoWidth(), src->profile.getVideoHeight() );
			return input;
		}
		case Frame::RGB: {
			input = new FlatInput( input_format, FORMAT_BGR, GL_UNSIGNED_BYTE, src->profile.getVideoWidth(), src->profile.getVideoHeight() );
			return input;
		}
		case Frame::GLSL: {
			input = new BlankInput( src->profile.getVideoWidth(), src->profile.getVideoHeight() );
			return input;
		}
	}
	return NULL;
}



QString MovitInput::getDescriptor( Frame *src )
{
	switch ( src->type() ) {
		case Frame::YUV420P:
			return QString("YCBCRINPUT %1 %2 %3 %4 %5 %6 %7 %8").arg( src->profile.getVideoWidth() ).arg( src->profile.getVideoHeight() ).arg( "yuv420p" )
			.arg( src->profile.getVideoColorSpace() ).arg( src->profile.getVideoColorPrimaries() ).arg( src->profile.getVideoColorFullRange() )
			.arg( src->profile.getVideoChromaLocation() ).arg( src->profile.getVideoGammaCurve() );
		case Frame::YUV422P:
			return QString("YCBCRINPUT %1 %2 %3 %4 %5 %6 %7 %8").arg( src->profile.getVideoWidth() ).arg( src->profile.getVideoHeight() ).arg( "yuv422p" )
			.arg( src->profile.getVideoColorSpace() ).arg( src->profile.getVideoColorPrimaries() ).arg( src->profile.getVideoColorFullRange() )
			.arg( src->profile.getVideoChromaLocation() ).arg( src->profile.getVideoGammaCurve() );
		case Frame::YUV420P10LE:
			return QString("YCBCRINPUT %1 %2 %3 %4 %5 %6 %7 %8").arg( src->profile.getVideoWidth() ).arg( src->profile.getVideoHeight() ).arg( "yuv420p10le" )
			.arg( src->profile.getVideoColorSpace() ).arg( src->profile.getVideoColorPrimaries() ).arg( src->profile.getVideoColorFullRange() )
			.arg( src->profile.getVideoChromaLocation() ).arg( src->profile.getVideoGammaCurve() );
		case Frame::YUV422P10LE:
			return QString("YCBCRINPUT %1 %2 %3 %4 %5 %6 %7 %8").arg( src->profile.getVideoWidth() ).arg( src->profile.getVideoHeight() ).arg( "yuv422p10le" )
			.arg( src->profile.getVideoColorSpace() ).arg( src->profile.getVideoColorPrimaries() ).arg( src->profile.getVideoColorFullRange() )
			.arg( src->profile.getVideoChromaLocation() ).arg( src->profile.getVideoGammaCurve() );
		case Frame::YUV420P12LE:
			return QString("YCBCRINPUT %1 %2 %3 %4 %5 %6 %7 %8").arg( src->profile.getVideoWidth() ).arg( src->profile.getVideoHeight() ).arg( "yuv420p12le" )
			.arg( src->profile.getVideoColorSpace() ).arg( src->profile.getVideoColorPrimaries() ).arg( src->profile.getVideoColorFullRange() )
			.arg( src->profile.getVideoChromaLocation() ).arg( src->profile.getVideoGammaCurve() );
		case Frame::YUV422P12LE:
			return QString("YCBCRINPUT %1 %2 %3 %4 %5 %6 %7 %8").arg( src->profile.getVideoWidth() ).arg( src->profile.getVideoHeight() ).arg( "yuv422p12le" )
			.arg( src->profile.getVideoColorSpace() ).arg( src->profile.getVideoColorPrimaries() ).arg( src->profile.getVideoColorFullRange() )
			.arg( src->profile.getVideoChromaLocation() ).arg( src->profile.getVideoGammaCurve() );
		case Frame::RGB:
			return QString("FLATINPUT RGB %1 %2").arg( src->profile.getVideoWidth() ).arg( src->profile.getVideoHeight() );
		case Frame::RGBA:
			return QString("FLATINPUT RGBA %1 %2").arg( src->profile.getVideoWidth() ).arg( src->profile.getVideoHeight() );
		case Frame::GLSL:
			return QString("BLANKINPUT %1 %2").arg( src->profile.getVideoWidth() ).arg( src->profile.getVideoHeight() );
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
