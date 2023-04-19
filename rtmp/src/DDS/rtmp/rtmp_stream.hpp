#ifndef RTMP_STREAM_HPP
#define RTMP_STREAM_HPP

#include <string>
#include <memory>
#include <vector>
#include <cstdint>
#include <unordered_map>

#include <DDS/core/client.hpp>
#include <DDS/core/tcp_session.hpp>
#include <DDS/rtmp/gop_cache.hpp>
#include <DDS/rtmp/flv_demuxer.hpp>
#include <DDS/rtmp/rtmp_chunk_stream.hpp>

static constexpr uint32_t MEDIA_STREAM_ID = 30;

class rtmp_stream
{
public:
	rtmp_stream(std::string url_)
		: url(url_), app(url_.substr(0, url_.find('/'))), cid(cid_from_hex(url_.substr(url_.find('/') + 1))) {}

	const uint32_t id = MEDIA_STREAM_ID;
	const std::string url, app;
	const ClientID_t cid;


	int join(std::shared_ptr<tcp_session>, bool publisher = false);
	void leave(std::shared_ptr<tcp_session>);
	bool has_publisher() const { return bool(publisher_); }
	unsigned count() const { return peers_.size(); }

	void send(std::shared_ptr<tcp_session>, std::shared_ptr<rtmp_chunk_stream>, uint16_t csid);
private:
	std::unordered_map<std::shared_ptr<tcp_session>, bool> peers_;
	gop_cache gop;
	flv_demuxer_wrap demux;

	std::shared_ptr<tcp_session> publisher_;
};

#endif