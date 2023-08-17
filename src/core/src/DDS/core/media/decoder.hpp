#ifndef MEDIA_DECODER_HPP
#define MEDIA_DECODER_HPP

class AVCodec;
class AVCodecContext;
class AVPacket;
class AVFrame;
class AVDictionary;
class media_decoder
{
public:
    media_decoder(int av_codec_id);
	virtual ~media_decoder();

    int open(AVDictionary** opt = nullptr);
    int decode(AVPacket*);

    //AVCodecContext* codec_context() const { return cc_; }
protected:
    virtual void on_decoded(AVFrame*) {}

	const AVCodec* c_;
	AVCodecContext* cc_ = nullptr;
private:
    AVFrame* fr_ = nullptr;
};

#endif // !MEDIA_ENCODER_HPP
