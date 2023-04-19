#ifndef MEDIA_ENCODER_HPP
#define MEDIA_ENCODER_HPP

class AVCodec;
class AVCodecContext;
class AVPacket;
class AVFrame;
class AVDictionary;
class media_encoder
{
public:
	media_encoder(int av_codec_id);
	virtual ~media_encoder();

    int open(AVDictionary** opt = nullptr);
    int encode(AVFrame*);
protected:
    virtual void on_encoded(AVPacket*) {}

	const AVCodec* c_;
	AVCodecContext* cc_ = nullptr;
private:
    AVPacket* pkt_ = nullptr;
};

#endif // !MEDIA_ENCODER_HPP
