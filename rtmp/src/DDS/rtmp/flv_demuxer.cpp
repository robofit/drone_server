#include <DDS/rtmp/flv_demuxer.hpp>
#include <DDS/core/media/manager.hpp>

extern "C"
{
#include <libavcodec/avcodec.h>
}


inline static
int flv_read_packet(std::shared_ptr<media_packet> pkt_ptr, AVCodecContext* cc, AVPacket* pkt)
{
    int ret;
    size_t size = pkt_ptr->data_p->size();
    uint8_t* pb = reinterpret_cast<uint8_t*>(pkt_ptr->data_p->data());

    pb++;
    size--;

    //if (pkt_ptr->is_key_frame_)
        //ret = av_add_index_entry(st, 0, pkt_ptr->dts_, size, 0, AVINDEX_KEYFRAME);

    pb += 4;
    size -= 4;

    if (pkt_ptr->seq_header_)
    {
        av_free(cc->extradata);
        cc->extradata = static_cast<uint8_t*>(av_mallocz(size + AV_INPUT_BUFFER_PADDING_SIZE));
        cc->extradata_size = size;
        memcpy(cc->extradata, pb, size);

        ret = AVERROR(EAGAIN);
    }
    else
    {
        if (!size)
            ret = AVERROR(EAGAIN);
        else
        {
            pkt->data = static_cast<uint8_t*>(av_realloc(pkt->data, size + AV_INPUT_BUFFER_PADDING_SIZE));
            memcpy(pkt->data, pb, size);
            ret = size;

            pkt->pos = -1;
            pkt->size = size;
            pkt->dts = pkt_ptr->dts_;
            pkt->pts = pkt_ptr->pts_;

            if (pkt_ptr->key_frame_)
                pkt->flags != AV_PKT_FLAG_KEY;

        }
    }

    return ret;
}

flv_demuxer::flv_demuxer(int av_codec_id, ClientID_t client_id)
    : media_decoder(av_codec_id), cid(client_id)
{
    pkt = av_packet_alloc();
}

flv_demuxer::~flv_demuxer()
{
    av_packet_unref(pkt);
    av_packet_free(&pkt);
}

int flv_demuxer::decode(std::shared_ptr<media_packet> pkt_ptr)
{
    if (pkt_ptr->type_ != media_packet::Type::Video)
        return 0;

    if (!media_manager::get().pipe(cid)->count())
        return 0;

    int ret;
    if (!avcodec_is_open(cc_))
    {
        ret = flv_read_packet(pkt_ptr, cc_, pkt);
        open();
    }
    else
    {
        ret = flv_read_packet(pkt_ptr, cc_, pkt);
    }


    if (pkt->size)
        media_decoder::decode(pkt);

	return 0;
}

void flv_demuxer::on_decoded(AVFrame* frame)
{
    auto pipe = media_manager::get().pipe(cid);

    pipe->write_frame(frame);
}


int flv_demuxer_wrap::write(std::shared_ptr<media_packet> pkt_ptr)
{
    if (!demux)
    {
        if (pkt_ptr->codec_ == media_packet::Codec::H264)
            demux = new flv_demuxer(AVCodecID::AV_CODEC_ID_H264, pkt_ptr->rtmp.cid_);
        else if (pkt_ptr->codec_ == media_packet::Codec::H265)
            demux = new flv_demuxer(AVCodecID::AV_CODEC_ID_H265, pkt_ptr->rtmp.cid_);
    }
    return demux->decode(pkt_ptr);
}
