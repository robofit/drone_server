#include <DDS/core/media/decoder.hpp>

extern "C"
{
#include <libavcodec/avcodec.h>
}

media_decoder::media_decoder(int av_codec_id)
{
    c_ = avcodec_find_decoder(static_cast<AVCodecID>(av_codec_id));
    cc_ = avcodec_alloc_context3(c_);
    fr_ = av_frame_alloc();
}

media_decoder::~media_decoder()
{
    if(avcodec_is_open(cc_))
        decode(nullptr);//flush

    av_frame_unref(fr_);
    av_frame_free(&fr_);
    avcodec_free_context(&cc_);
}

int media_decoder::open(AVDictionary** opt)
{
    return avcodec_is_open(cc_) ? -1 : avcodec_open2(cc_, c_, opt);
}

int media_decoder::decode(AVPacket* pkt)
{
    int ret = avcodec_send_packet(cc_, pkt);
    while (ret >= 0)
    {
        ret = avcodec_receive_frame(cc_, fr_);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
            break;
        else if (ret < 0)
            return -1;

        on_decoded(fr_);
    }
    return ret;
}
