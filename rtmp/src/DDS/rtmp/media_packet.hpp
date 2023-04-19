#ifndef MEDIA_PACKET_HPP
#define MEDIA_PACKET_HPP

#include <string>
#include <memory>
#include <cstdint>
#include <vector>

#include <DDS/core/client.hpp>

struct media_packet
{
	struct
	{
		uint32_t stream_id_;
		uint8_t type_;
		std::string url_;
		std::string app_;
		ClientID_t cid_;
	} rtmp;

	enum class Type
	{
		Unknown = -1,
		Video,
		Audio,
		Metadata
	} type_ = Type::Unknown;

	enum class Codec
	{
		Unknown = -1,
		H264,
		H265,
		VP8,
		VP9,
		AAC,
		Opus,
		MP3
	} codec_ = Codec::Unknown;

	int64_t dts_ = 0;
	int64_t pts_ = 0;

	bool key_frame_ = false;
	bool seq_header_ = false;

	std::shared_ptr<std::vector<uint8_t>> data_p;
};


#endif // !MEDIA_PACKET_HPP
