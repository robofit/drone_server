#include <DDS/rtmp/rtmp_stream.hpp>
#include <DDS/rtmp/rtmp_session.hpp>

#include <DDS/core/media/manager.hpp>

int rtmp_stream::join(std::shared_ptr<tcp_session> session, bool publisher)
{
    if(publisher_ && publisher)
        return -1;
    
	peers_.insert(std::make_pair(session, false));
    if(publisher)
        publisher_ = session;
    return 0;
}

void rtmp_stream::leave(std::shared_ptr<tcp_session> session)
{
    if(session == publisher_)
    {
        media_manager::get().erase(cid);
        publisher_.reset();

        peers_.erase(session);

        for (auto& sp : peers_)
        {
            std::dynamic_pointer_cast<rtmp_session>(sp.first)->send_stream_eof(id);
        }
    }
    else
    {
        peers_.erase(session);
    }
}

inline
std::shared_ptr<media_packet> chunk_stream_to_media_pkt(rtmp_stream* s, std::shared_ptr<rtmp_chunk_stream> cs)
{
    std::shared_ptr<media_packet> pkt = std::make_shared<media_packet>();

    pkt->rtmp.type_ = cs->type();
    pkt->rtmp.stream_id_ = cs->stream();
    pkt->rtmp.url_ = s->url;
    pkt->rtmp.app_ = s->url.substr(0, s->url.find('/'));
    pkt->rtmp.cid_ = cid_from_hex(s->url.substr(s->url.find('/') + 1));

    uint8_t* data = cs->data()->data();

    uint32_t delta_t = 0;
    if (pkt->rtmp.type_ == 9)//video
    {
        pkt->type_ = media_packet::Type::Video;

        uint8_t codec = data[0] & 0x0f;
        if (codec == 0x07)
            pkt->codec_ = media_packet::Codec::H264;
        else if(codec == 0x0c)
            pkt->codec_ = media_packet::Codec::H265;
        else if (codec == 0x0e)
            pkt->codec_ = media_packet::Codec::VP8;
        else if (codec == 0x0f)
            pkt->codec_ = media_packet::Codec::VP9;

        uint8_t frame_type = data[0] & 0xf0;
        uint8_t nalu = data[1];

        if (frame_type == 0x10)//key
        {
            if (nalu == 0)//seq
                pkt->seq_header_ = true;
            else if (nalu == 1)
                pkt->key_frame_ = true;
        }
        else if (frame_type == 0x20)
            pkt->key_frame_ = false;

        delta_t = (data[4] << 16) | (data[3] << 8) | data[2];
    }
    else if (pkt->rtmp.type_ == 8)//audio
    {
        pkt->type_ = media_packet::Type::Audio;

        uint8_t codec = data[0] & 0xf0;
        if (codec == 0xa0)
        {
            pkt->codec_ = media_packet::Codec::AAC;
            if (data[1] == 0)
            {
                pkt->seq_header_ = true;
            }
            else if (data[1] == 1)
            {
                pkt->seq_header_ = false;
                pkt->key_frame_ = false;
            }
        }
        else if (codec == 0x90)
        {
            pkt->codec_ = media_packet::Codec::Opus;
            if (data[1] == 0)
            {
                pkt->seq_header_ = true;
            }
            else if (data[1] == 1)
            {
                pkt->seq_header_ = false;
                pkt->key_frame_ = false;
            }
        }
    }
    else if (pkt->rtmp.type_ == 18 || pkt->rtmp.type_ == 15)//metadata
    {
        pkt->type_ = media_packet::Type::Metadata;
    }

    pkt->dts_ = cs->time();
    pkt->pts_ = pkt->dts_ + delta_t;
    pkt->data_p = std::shared_ptr<std::vector<uint8_t>>(new std::vector<uint8_t>(*cs->data()));

    return pkt;
}



void rtmp_stream::send(std::shared_ptr<tcp_session> session, std::shared_ptr<rtmp_chunk_stream> cs, uint16_t csid)
{
    if(session != publisher_)
        return;

    auto pkt = chunk_stream_to_media_pkt(this, cs);
    gop.insert(pkt);
    demux.write(pkt);

    for (auto& sp : peers_)
    {
        if (sp.first != session)
        {
            if (!sp.second)
            {
                peers_[sp.first] = true;
                gop.write(std::dynamic_pointer_cast<rtmp_session>(sp.first));
            }
            else
            {
                std::dynamic_pointer_cast<rtmp_session>(sp.first)->write_media_packet(pkt);
            }
        }
    }
}
