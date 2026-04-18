#include <QDebug>
#include <QElapsedTimer>

#include "output_ff.h"



OutputFF::OutputFF( MQueue<Frame*> *vf, MQueue<Frame*> *af )
	: audioFrames( af ),
	videoFrames( vf ),
	running( false ),
	formatCtx( NULL ),
	videoStream( NULL ),
	videoCodecCtx( NULL ),
	videoFrame( NULL ),
	audioStream( NULL ),
	audioCodecCtx( NULL ),
	audioFrame( NULL ),
	audioSamples( NULL ),
	audioSamplesSize( 0 ),
	audioBuffer( NULL ),
	audioBufferLen( 0 ),
	swr( NULL ),
	endPTS( 0 ),
	showFrameProgress( true )
{
	FFmpegCommon::getGlobalInstance()->initFFmpeg();
}



OutputFF::~OutputFF()
{
	close();
}



void OutputFF::close()
{
    // 2. On libère les contextes de codecs (les nôtres, pas ceux du stream)
    if ( videoCodecCtx ) {
        avcodec_free_context( &videoCodecCtx );
        videoCodecCtx = NULL;
    }
    if ( audioCodecCtx ) {
        avcodec_free_context( &audioCodecCtx );
        audioCodecCtx = NULL;
    }

    // 3. On libère les structures de format (les streams sont libérés avec)
    if ( formatCtx ) {
        if ( !(formatCtx->oformat->flags & AVFMT_NOFILE) ) {
            avio_closep( &formatCtx->pb );
        }
        avformat_free_context( formatCtx );
        formatCtx = NULL;
    }

    // 4. On remet les pointeurs de streams à NULL (ils ont été libérés par avformat_free_context)
    videoStream = NULL;
    audioStream = NULL;

    // 5. Libération des frames et buffers
    if ( videoFrame ) {
		av_frame_free( &videoFrame );
		videoFrame = NULL;
	}
    if ( audioFrame ) {
		av_frame_free( &audioFrame );
		audioFrame = NULL;
	}
    
    if ( swr ) {
		swr_free( &swr );
		swr = NULL;
	}
    
    if ( audioBuffer ) {
		av_freep( &audioBuffer );
		audioBufferLen = 0;
	}
}



bool OutputFF::openVideo( Profile &prof, int vrate, int vcodec, QString vcodecName )
{
	AVCodecID vcodec_id = AV_CODEC_ID_H264;
	switch (vcodec) {
		case VCODEC_HEVC: vcodec_id = AV_CODEC_ID_H265; break;
		case VCODEC_MPEG2: vcodec_id = AV_CODEC_ID_MPEG2VIDEO; break;
	}
	const AVCodec *codec = NULL;
	if (!vcodecName.isEmpty() && vcodecName != "default") {
		codec = avcodec_find_encoder_by_name(vcodecName.toUtf8().data());
	}
	if (!codec) {
		codec = avcodec_find_encoder( vcodec_id );
	}
	if ( !codec ) {
		qDebug() << "Could not find video encoder.";
		return false;
	}
	videoStream = avformat_new_stream( formatCtx, NULL );
	if ( !videoStream ) {
		qDebug() << "Could not allocate video stream.";
		return false;
	}
	videoStream->id = formatCtx->nb_streams - 1;
	
	videoCodecCtx = avcodec_alloc_context3(codec);
	if (!videoCodecCtx) {
		qDebug() << "Could not allocate video codec context.";
		return false;
	}

	/* put sample parameters */
	videoCodecCtx->bit_rate = vrate * 1000000;
	/* resolution must be a multiple of two */
	videoCodecCtx->width = prof.getVideoWidth();
	videoCodecCtx->height = prof.getVideoHeight();
	/* frames per second */
	videoCodecCtx->time_base = (AVRational){1,25};
	double fps = prof.getVideoFrameRate();
	for ( int i = 0; i < NCFR; ++i ) {
		double cfps = CommonFrameRates[i][0] / CommonFrameRates[i][1];
		if ( qAbs( fps - cfps ) < 1e-3 ) {
			videoCodecCtx->time_base = (AVRational){ (int)CommonFrameRates[i][1], (int)CommonFrameRates[i][0] };
			break;
		}
	}
	videoCodecCtx->gop_size = prof.getVideoFrameRate();
	videoCodecCtx->max_b_frames = 2;
	videoCodecCtx->pix_fmt = AV_PIX_FMT_YUV420P;
	videoCodecCtx->thread_count = 0; // auto : FFmpeg choisit le nombre optimal de threads
	int sw = (double)prof.getVideoWidth() * prof.getVideoSAR();
	int sh = prof.getVideoWidth();
	videoCodecCtx->sample_aspect_ratio = (AVRational){ sw, sh };
	
	videoStream->sample_aspect_ratio = videoCodecCtx->sample_aspect_ratio;
	videoStream->time_base = videoCodecCtx->time_base;

	//if ( codec_id == AV_CODEC_ID_H264 )
		//av_opt_set( videoCodecCtx->priv_data, "preset", "veryslow", 0 );
	
	if ( formatCtx->oformat->flags & AVFMT_GLOBALHEADER )
		videoCodecCtx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

	// open it
	if ( avcodec_open2( videoCodecCtx, codec, NULL ) < 0 ) {
		qDebug() << "Could not open video codec.";
		return false;
	}

	if (avcodec_parameters_from_context(videoStream->codecpar, videoCodecCtx) < 0) {
		qDebug() << "Could not copy video codec parameters";
		return false;
	}

	videoFrame = av_frame_alloc();
	if ( !videoFrame ) {
		qDebug() << "Could not allocate video frame.";
		return false;
	}
	videoFrame->format = videoCodecCtx->pix_fmt;
	videoFrame->width  = videoCodecCtx->width;
	videoFrame->height = videoCodecCtx->height;

	// allocate the buffers for the frame data
	int ret = av_frame_get_buffer( videoFrame, 32 );
	if ( ret < 0 ) {
		qDebug() << "Could not allocate video frame buffers.";
		return false;
	}
	
	return true;
}



bool OutputFF::openAudio( Profile &prof, int vcodec )
{
    // Initialisation par sécurité pour éviter les crashs dans close() en cas d'échec ici
    audioStream = NULL;
    audioCodecCtx = NULL;
    audioFrame = NULL;
    swr = NULL;
    audioBuffer = NULL;

	const AVCodec *codec;

    // 1. Sélection du codec
    AVCodecID codec_id = (vcodec == VCODEC_HEVC || vcodec == VCODEC_H264) ? AV_CODEC_ID_AAC : AV_CODEC_ID_MP2;
    codec = avcodec_find_encoder(codec_id);
    if (!codec) {
        qDebug() << "Audio encoder not found";
        return false;
    }

    // 2. Création du flux
    audioStream = avformat_new_stream(formatCtx, NULL);
    if (!audioStream) return false;
    audioStream->id = formatCtx->nb_streams - 1;

    // 3. Allocation du contexte
    audioCodecCtx = avcodec_alloc_context3(codec);
    if (!audioCodecCtx) return false;

    // 4. Paramétrage (Ajusté pour éviter les incompatibilités)
    audioCodecCtx->sample_fmt     = codec->sample_fmts ? codec->sample_fmts[0] : AV_SAMPLE_FMT_FLTP;
    audioCodecCtx->bit_rate       = 128000;
    audioCodecCtx->sample_rate    = prof.getAudioSampleRate();
    av_channel_layout_from_mask( &audioCodecCtx->ch_layout, AV_CH_LAYOUT_STEREO );
    
    // TRÈS IMPORTANT : évite le crash au trailer pour certains formats (MP4)
    if (formatCtx->oformat->flags & AVFMT_GLOBALHEADER) {
        audioCodecCtx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    }

    // 5. Ouverture du codec
    if (avcodec_open2(audioCodecCtx, codec, NULL) < 0) return false;

    // 6. Transfert des paramètres vers le flux (Essentiel pour av_write_trailer)
    avcodec_parameters_from_context(audioStream->codecpar, audioCodecCtx);

    // 7. ALLOCATION DU BUFFER
    int inChannels = prof.getAudioChannels();
    int inBytesPerChannel = Profile::bytesPerChannel(&prof);
    int inBytesPerSample = inChannels * inBytesPerChannel;

    audioCodecFrameSize = audioCodecCtx->frame_size;
    if (audioCodecFrameSize <= 0) audioCodecFrameSize = 1024;

    // Allocation sécurisée du buffer intermédiaire
    audioBuffer = (uint8_t*)av_malloc(audioCodecFrameSize * inBytesPerSample);
    audioBufferLen = 0;
    if (!audioBuffer) return false;

    // 8. Préparation de la Frame FFmpeg
    audioFrame = av_frame_alloc();
    if (!audioFrame) return false;
    audioFrame->nb_samples     = audioCodecFrameSize;
    audioFrame->format         = audioCodecCtx->sample_fmt;
    av_channel_layout_copy( &audioFrame->ch_layout, &audioCodecCtx->ch_layout );
    
    if (av_frame_get_buffer(audioFrame, 0) < 0) return false;

    // 9. Initialiser le Resampler (SWR) avec détection précise
    AVChannelLayout in_ch_layout = {};
    av_channel_layout_from_mask( &in_ch_layout, (inChannels == 1) ? AV_CH_LAYOUT_MONO : AV_CH_LAYOUT_STEREO );

    AVSampleFormat in_sample_fmt;
    if (inBytesPerChannel == 2) {
        in_sample_fmt = AV_SAMPLE_FMT_S16;
    } else {
        in_sample_fmt = AV_SAMPLE_FMT_FLT;
    }

    swr_alloc_set_opts2( &swr,
                         &audioCodecCtx->ch_layout, audioCodecCtx->sample_fmt, audioCodecCtx->sample_rate,
                         &in_ch_layout,             in_sample_fmt,             prof.getAudioSampleRate(),
                         0, NULL );
    av_channel_layout_uninit( &in_ch_layout );
    
    if (!swr || swr_init(swr) < 0) {
        qDebug() << "Could not initialize the resampling context";
        return false;
    }

    totalSamples = 0;
    return true;
}



bool OutputFF::openFormat( QString filename, Profile &prof, int vrate, int vcodec, QString vcodecName )
{
	int ret;
	QString container;
	
	switch (vcodec) {
		case VCODEC_HEVC: {
			container = "matroska";
			break;
		}
		case VCODEC_MPEG2: {
			container = "mpeg";
			break;
		}
		default: {
			container = "mp4";
		}
	}
	avformat_alloc_output_context2( &formatCtx, NULL, container.toLatin1().data(), filename.toLatin1().data() );
	
	if ( !formatCtx ) {
		qDebug() << "Could not open format context.";
		return false;
	}
	
	if ( !openVideo( prof, vrate, vcodec, vcodecName ) ) {
		close();
		return false;
	}

	if ( !openAudio( prof, vcodec ) ) {
		close();
		return false;
	}
	
	// open the output file, if needed
	if ( !( formatCtx->oformat->flags & AVFMT_NOFILE ) ) {
		ret = avio_open( &formatCtx->pb, filename.toLatin1().data(), AVIO_FLAG_WRITE );
		if ( ret < 0 ) {
			qDebug() << "Could not open" << filename;
			close();
			return false;
		}
	}

	if (formatCtx->nb_streams != 2) {
		qDebug() << "Unexpected number of streams in format context:" << formatCtx->nb_streams;
		close();
		return false;
	}
    
	// Write the stream header, if any.
	ret = avformat_write_header( formatCtx, NULL );
	if (ret < 0) {
		qDebug() << "Error occurred when opening output file.";
		close();
		return false;
	}
	
	return true;
}



bool OutputFF::init( QString filename, Profile &prof, int vrate, int vcodec, QString vcodecName, double end )
{
	close();

	if ( !audioFrames || !videoFrames )
		return false;

	if ( !openFormat( filename, prof, vrate, vcodec, vcodecName ) )
		return false;	

	endPTS = end;

	return true;
}



void OutputFF::startEncode( bool show )
{
	showFrameProgress = show;
	running = true;
	start();
}



bool OutputFF::cancel()
{
	running = false;
	wait();
	return true;
}



void OutputFF::run()
{
	Frame *f;
	unsigned nVideo = 0, nAudio = 0;
	bool wait;
	bool videoEnd = false;
	QElapsedTimer time;
	time.start();
	totalSamples = 0;
	
	while ( running ) {
		wait = true;
		if ( !videoEnd && nVideo == nAudio ) {
			if ( (f = videoFrames->dequeue()) ) {
				encodeVideo( f, nVideo );
				//qDebug() << "N:" << nVideo << f->pts();
				if ( f->pts() > endPTS )
					videoEnd = true;
				if ( showFrameProgress && time.elapsed() >= 1000 ) {
					emit showFrame( f );
					time.restart();
				}
				else
					f->release();
				++nVideo;
				wait = false;
			}
		}
		if ( nAudio < nVideo ) {
			if ( (f = audioFrames->dequeue()) ) {
				encodeAudio( f, nAudio );
				//qDebug() << "N audio:" << nAudio;
				f->release();
				++nAudio;
				wait = false;
			}
		}
		
		if ( videoEnd && nVideo == nAudio ) {
			qDebug() << "OutputFF::run break";
			break;
		}
		if ( wait ) {
			//qDebug() << "OutputFF::run sleep";
			usleep( 1000 );
		}
	}

	// Flush Vidéo
	AVPacket *pkt = av_packet_alloc(); // Allocation unique pour tout le flush
	avcodec_send_frame(videoCodecCtx, NULL);
	while (true) {
		int ret = avcodec_receive_packet(videoCodecCtx, pkt);
		if (ret == AVERROR_EOF || ret == AVERROR(EAGAIN))
			break;

		av_packet_rescale_ts(pkt, videoCodecCtx->time_base, videoStream->time_base);
		pkt->stream_index = videoStream->index;
		av_interleaved_write_frame(formatCtx, pkt);
		av_packet_unref(pkt);
	}

	// Flush Audio
	avcodec_send_frame(audioCodecCtx, NULL);
	while (true) {
		int ret = avcodec_receive_packet(audioCodecCtx, pkt);
		if (ret == AVERROR_EOF || ret == AVERROR(EAGAIN))
			break;

		av_packet_rescale_ts(pkt, audioCodecCtx->time_base, audioStream->time_base);
		pkt->stream_index = audioStream->index;
		av_interleaved_write_frame(formatCtx, pkt);
		av_packet_unref(pkt);
	}

	// Libération finale
	av_packet_free(&pkt);

	/* Write the trailer, if any. The trailer must be written before you
	* close the CodecContexts open when you wrote the header; otherwise
	* av_write_trailer() may try to use memory that was freed on
	* av_codec_close(). */
	av_write_trailer( formatCtx );

	close();
}



bool OutputFF::encodeVideo( Frame *f, int nFrame )
{
    if ( !videoStream || !videoCodecCtx || !videoFrame ) return false;

    int ret;
    AVPacket *pkt = av_packet_alloc();
    if ( !pkt ) return false;

    // 1. Préparation de la frame
    if ( av_frame_make_writable( videoFrame ) < 0 ) {
        av_packet_free( &pkt );
        return false;
    }

    // 2. COPIE DES PIXELS (YUV420P)
    int width = videoCodecCtx->width;
    int height = videoCodecCtx->height;
    uint8_t *src = f->data();

    // Plan Y
    for ( int y = 0; y < height; y++ ) {
        memcpy( videoFrame->data[0] + y * videoFrame->linesize[0], src + y * width, width );
    }
    src += width * height;

    // Plan U
    for ( int y = 0; y < height / 2; y++ ) {
        memcpy( videoFrame->data[1] + y * videoFrame->linesize[1], src + y * ( width / 2 ), width / 2 );
    }
    src += ( width / 2 ) * ( height / 2 );

    // Plan V
    for ( int y = 0; y < height / 2; y++ ) {
        memcpy( videoFrame->data[2] + y * videoFrame->linesize[2], src + y * ( width / 2 ), width / 2 );
    }

    // 3. RÉGLAGE DU PTS
    // Comme la time_base est réglée sur 1/fps dans openVideo, le PTS est simplement l'index de l'image.
    videoFrame->pts = nFrame;

    // 4. ENCODAGE
    ret = avcodec_send_frame( videoCodecCtx, videoFrame );
    if ( ret < 0 ) {
        qDebug() << "Error sending video frame to encoder" << nFrame;
        av_packet_free( &pkt );
        return false;
    }

    while ( ret >= 0 ) {
        ret = avcodec_receive_packet( videoCodecCtx, pkt );
        if ( ret == AVERROR(EAGAIN) || ret == AVERROR_EOF ) break;
        if ( ret < 0 ) {
            av_packet_free( &pkt );
            return false;
        }

        // Rescale des timestamps de l'encodeur vers le conteneur
        av_packet_rescale_ts( pkt, videoCodecCtx->time_base, videoStream->time_base );
        pkt->stream_index = videoStream->index;

        ret = av_interleaved_write_frame( formatCtx, pkt );
        av_packet_unref( pkt );
        
        if ( ret < 0 ) qDebug() << "Error while writing video frame PTS:" << nFrame;
    }
    
    av_packet_free( &pkt );
    return true;
}



bool OutputFF::encodeAudio( Frame *f, int nFrame )
{
    if ( !audioStream || !audioCodecCtx || !swr || !audioBuffer ) return false;

    int ret;

    AVPacket *pkt = av_packet_alloc();
    if ( !pkt ) return false;

    // Récupération des données brutes de la frame entrante
    uint8_t *buf = f->data();
    int nSamples = f->audioSamples();

    // Calcul de la taille d'un échantillon complet en octets (ex: 2 canaux * 4 bytes = 8 bytes)
    // On utilise les infos du profil pour être cohérent avec ce que openAudio a configuré
    int inBytesPerSample = f->profile.getAudioChannels() * Profile::bytesPerChannel(&f->profile);
    
    // Pointeur de fin pour la boucle de lecture
    uint8_t *end = buf + (nSamples * inBytesPerSample);

    do {
        // On remplit le buffer tampon jusqu'à avoir assez pour une frame AAC (généralement 1024 samples)
        // audioCodecFrameSize a été défini dans openAudio
        int samplesNeeded = audioCodecFrameSize - (audioBufferLen / inBytesPerSample);
        int samplesAvailable = nSamples;
        
        int ns = qMin( samplesAvailable, samplesNeeded );
        
        // Copie des données dans le buffer intermédiaire
        if ( ns > 0 ) {
            memcpy( audioBuffer + audioBufferLen, buf, ns * inBytesPerSample );
            audioBufferLen += ns * inBytesPerSample;
            nSamples -= ns;
            buf += ns * inBytesPerSample;
        }

        // Si le buffer est plein (on a 1024 samples), on encode
        if ( audioBufferLen == audioCodecFrameSize * inBytesPerSample ) {
            
            // A. CONVERSION (Interleaved -> Planar / S16 -> FLTP)
            const uint8_t *inData[1];
            inData[0] = audioBuffer;

            // On s'assure que la frame de sortie est inscriptible
            ret = av_frame_make_writable( audioFrame );
            if ( ret < 0 ) {
                av_packet_free( &pkt );
                return false;
            }

            // Conversion via SwrContext
            // Note : swr_convert attend le nombre d'échantillons PAR CANAL, pas les octets.
            ret = swr_convert( swr, 
                         audioFrame->data,          audioCodecFrameSize,
                         (const uint8_t**)inData,   audioCodecFrameSize );
            
            if ( ret < 0 ) {
                 qDebug() << "Error during swr_convert";
                 av_packet_free( &pkt );
                 return false;
            }

            // B. CALCUL DU PTS (Presentation Time Stamp)
            // Utilise totalSamples pour garantir la continuité temporelle sans trous
            audioFrame->pts = av_rescale_q( totalSamples,
                                            (AVRational){1, audioCodecCtx->sample_rate},
                                            audioCodecCtx->time_base );

            // C. ENCODAGE (Nouvelle API Send/Receive)
            // Envoi de la frame brute à l'encodeur
            ret = avcodec_send_frame( audioCodecCtx, audioFrame );
            if ( ret < 0 ) {
                qDebug() << "Error sending audio frame to encoder (nFrame:" << nFrame << ") Code:" << ret;
                // On ne retourne pas false ici, on essaie de continuer au cas où
            }

            // Récupération des paquets compressés
            while ( ret >= 0 ) {
                ret = avcodec_receive_packet( audioCodecCtx, pkt );
                
                if ( ret == AVERROR(EAGAIN) || ret == AVERROR_EOF ) {
                    break; 
                } else if ( ret < 0 ) {
                    qDebug() << "Error encoding audio frame (Receive)";
                    break;
                }

                // Ajustement des timestamps pour le conteneur (MP4/MKV...)
                av_packet_rescale_ts( pkt, audioCodecCtx->time_base, audioStream->time_base );
                pkt->stream_index = audioStream->index;
                
                // Écriture dans le fichier
                int writeRet = av_interleaved_write_frame( formatCtx, pkt );
                if ( writeRet < 0 ) {
                    qDebug() << "Error while writing audio packet.";
                }
                
                // Libération du contenu du paquet (mais pas de la structure)
                av_packet_unref( pkt );
            }

            // Mise à jour du compteur global et du buffer
            totalSamples += audioCodecFrameSize;
            audioBufferLen = 0;
        }
    } while ( buf < end );

    // Libération finale de la structure du paquet
    av_packet_free( &pkt );

    return true;
}
