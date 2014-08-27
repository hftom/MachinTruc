#include <movit/ycbcr_input.h>
#include <movit/flat_input.h>

#include "engine/movitchain.h"

#define BUFFER_OFFSET(i) ((uint8_t*)NULL + (i))



MovitInput::MovitInput()
{
	mmi = -1;
}



MovitInput::~MovitInput()
{
}



bool MovitInput::setBuffer( PBO *p, Frame *src, int size )
{
	glBindBuffer( GL_PIXEL_UNPACK_BUFFER_ARB, p->pbo() );
	void *mem = glMapBuffer( GL_PIXEL_UNPACK_BUFFER_ARB, GL_WRITE_ONLY );
	if ( !mem )
		return false;
	memcpy( mem, src->data(), size );
	glUnmapBuffer( GL_PIXEL_UNPACK_BUFFER_ARB );
	glBindBuffer( GL_PIXEL_UNPACK_BUFFER_ARB, 0 );
	src->setPBO( p );
	return true;
}



bool MovitInput::process( Frame *src, GLResource *gl )
{
	if ( mmi != -1 && src->mmi != 0 && src->mmi == mmi )
		return true;
	
	mmi = src->mmi;
	
	int w = src->profile.getVideoWidth();
	int h = src->profile.getVideoHeight();
	uint8_t *data = src->data();
	int size;

	switch ( src->type() ) {
		case Frame::YUV420P: {
			YCbCrInput *ycbcr = (YCbCrInput*)input;
			size = w * h * 3 / 2;
			PBO *p = NULL;
			if ( gl )
				p = gl->getPBO( size );
			if ( p && setBuffer( p, src, size ) ) {
				ycbcr->set_pixel_data( 0, BUFFER_OFFSET( 0 ), p->pbo() );
				ycbcr->set_pixel_data( 1, BUFFER_OFFSET( w * h ), p->pbo());
				ycbcr->set_pixel_data( 2, BUFFER_OFFSET( w * h + (w / 2 * h / 2) ), p->pbo() );
			}
			else {
				ycbcr->set_pixel_data( 0, data );
				ycbcr->set_pixel_data( 1, &data[w * h]);
				ycbcr->set_pixel_data( 2, &data[w * h + (w / 2 * h / 2)] );
			}
			return true;
		}
		case Frame::YUV422P: {
			YCbCrInput *ycbcr = (YCbCrInput*)input;
			size = w * h * 2;
			PBO *p = NULL;
			if ( gl )
				p = gl->getPBO( size );
			if ( p  && setBuffer( p, src, size ) ) {
				ycbcr->set_pixel_data( 0, BUFFER_OFFSET( 0 ), p->pbo() );
				ycbcr->set_pixel_data( 1, BUFFER_OFFSET( w * h ), p->pbo());
				ycbcr->set_pixel_data( 2, BUFFER_OFFSET( w * h + (w / 2 * h) ), p->pbo() );
			}
			else {
				ycbcr->set_pixel_data( 0, data );
				ycbcr->set_pixel_data( 1, &data[w * h]);
				ycbcr->set_pixel_data( 2, &data[w * h + (w / 2 * h)] );
			}
			return true;
		}
		case Frame::RGB:
		case Frame::RGBA: {
			FlatInput *flat = (FlatInput*)input;
			size = src->type() == Frame::RGB ? w * h * 3 : w * h * 4;
			PBO *p = NULL;
			if ( gl )
				p = gl->getPBO( size );
			if ( p && setBuffer( p, src, size ) ) {
				flat->set_pixel_data( BUFFER_OFFSET( 0 ), p->pbo() );
			}
			else
				flat->set_pixel_data( data );
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
			input = new YCbCrInput( input_format, ycbcr_format, src->profile.getVideoWidth(), src->profile.getVideoHeight() );
			return input;
		}
		case Frame::YUV422P: {
			ycbcr_format.chroma_subsampling_x = 2;
			ycbcr_format.chroma_subsampling_y = 1;
			input = new YCbCrInput( input_format, ycbcr_format, src->profile.getVideoWidth(), src->profile.getVideoHeight() );
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
		case Frame::RGB:
			return QString("FLATINPUT RGB %1 %2").arg( src->profile.getVideoWidth() ).arg( src->profile.getVideoHeight() );
		case Frame::RGBA:
			return QString("FLATINPUT RGBA %1 %2").arg( src->profile.getVideoWidth() ).arg( src->profile.getVideoHeight() );
	}
	return QString();
}



MovitFilter::MovitFilter( const QList<Effect*> &el, GLFilter *f, bool owns )
{
	effects = el;
	filter = f;
	ownsFilter = owns;
}
	
MovitFilter::~MovitFilter()
{
	if ( ownsFilter )
		filter->release();
}

MovitComposition::MovitComposition( Effect *e, GLComposition *c, bool owns )
{
	effect = e;
	composition = c;
	ownsComposition = owns;
}
	
MovitComposition::~MovitComposition()
{
	if ( ownsComposition )
		delete composition;
}

MovitBranch::MovitBranch( MovitInput *in )
{
	input = in;
	composition = NULL;
}
	
MovitBranch::~MovitBranch() 
{
	delete input;
	while ( !filters.isEmpty() )
		delete filters.takeFirst();
	if ( composition )
		delete composition;
}

MovitChain::MovitChain()
{
	chain = NULL;
	lastPTS = 0;
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
	lastPTS = -1;
}
