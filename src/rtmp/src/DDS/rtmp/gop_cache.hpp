#ifndef GOP_CACHE_HPP
#define GOP_CACHE_HPP

#include <vector>
#include <memory>
#include <DDS/rtmp/media_packet.hpp>
#include <DDS/rtmp/rtmp_session.hpp>

class gop_cache
{
public:
	gop_cache(unsigned min_gop = 1)
		: min_gop_(min_gop) {}

	int insert(std::shared_ptr<media_packet>);
	void write(std::shared_ptr<rtmp_session>);
private:
	std::vector<std::shared_ptr<media_packet>> pkts;
	std::shared_ptr<media_packet> video_header_pkt, audio_header_pkt, metadata_header_pkt;

	unsigned min_gop_;
	unsigned gop_count_ = 0;
};


#endif // !GOP_CACHE_HPP
