#ifndef FLV_DEMUXER_HPP
#define FLV_DEMUXER_HPP

#include <DDS/core/media/decoder.hpp>
#include <DDS/rtmp/media_packet.hpp>

class AVPacket;
class AVFrame;
class flv_demuxer : media_decoder
{
public:
    flv_demuxer(int av_codec_id, ClientID_t cid);
    ~flv_demuxer();

    int decode(std::shared_ptr<media_packet>);
private:
    const ClientID_t cid;
    AVPacket* pkt = nullptr;

    void on_decoded(AVFrame*);
};

class flv_demuxer_wrap
{
    flv_demuxer* demux = nullptr;
public:
    ~flv_demuxer_wrap() { delete demux; }
    int write(std::shared_ptr<media_packet>);
};


#endif 