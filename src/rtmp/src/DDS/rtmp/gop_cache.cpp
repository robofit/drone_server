#include <DDS/rtmp/gop_cache.hpp>

int gop_cache::insert(std::shared_ptr<media_packet> pkt)
{
    if (pkt->type_ == media_packet::Type::Video)
    {
        if (pkt->seq_header_)
        {
            video_header_pkt = pkt;
            return pkts.size();
        }

        if (pkt->key_frame_ && ((++gop_count_) % min_gop_) == 0)
        {
            pkts.clear();
        }
    }
    else if (pkt->type_ == media_packet::Type::Audio)
    {
        if (pkt->seq_header_)
        {
            audio_header_pkt = pkt;
            return pkts.size();
        }
    }
    else if (pkt->type_ == media_packet::Type::Metadata)
    {
        metadata_header_pkt = pkt;
        return pkts.size();
    }

    pkts.push_back(pkt);
    return pkts.size();
}

void gop_cache::write(std::shared_ptr<rtmp_session> tcp_s)
{
    if (metadata_header_pkt)
        tcp_s->write_media_packet(metadata_header_pkt);

    if (video_header_pkt)
        tcp_s->write_media_packet(video_header_pkt);

    if (audio_header_pkt)
        tcp_s->write_media_packet(audio_header_pkt);

    for (auto pkt : pkts)
        tcp_s->write_media_packet(pkt);
}
