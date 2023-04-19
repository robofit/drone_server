#include <DDS/core/media/encoder.hpp>

extern "C"
{
#include <libavcodec/avcodec.h>
}

media_encoder::media_encoder(int av_codec_id)
{
    c_ = avcodec_find_encoder(static_cast<AVCodecID>(av_codec_id));
    cc_ = avcodec_alloc_context3(c_);
    pkt_ = av_packet_alloc();
}

media_encoder::~media_encoder()
{
    if(avcodec_is_open(cc_))
        encode(nullptr);//flush

    av_packet_unref(pkt_);
    av_packet_free(&pkt_);
    avcodec_free_context(&cc_);
}

int media_encoder::open(AVDictionary** opt)
{
    return avcodec_is_open(cc_) ? -1 : avcodec_open2(cc_, c_, opt);
}

int media_encoder::encode(AVFrame* frame)
{
    int ret = avcodec_send_frame(cc_, frame);
    while (ret >= 0)
    {
        ret = avcodec_receive_packet(cc_, pkt_);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
            break;
        else if (ret < 0)
            return -1;

        on_encoded(pkt_);
    }
    return ret;
}
