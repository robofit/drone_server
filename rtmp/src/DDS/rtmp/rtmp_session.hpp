#ifndef RTMP_SESSION_HPP
#define RTMP_SESSION_HPP

#include <cstdint>
#include <string>
#include <unordered_map>
#include <memory>
#include <vector>

#include <DDS/core/tcp_session.hpp>
#include <DDS/rtmp/media_packet.hpp>
#include <DDS/rtmp/rtmp_chunk_stream.hpp>
#include <DDS/rtmp/rtmp_handshake.hpp>

class rtmp_session : public tcp_session
{
	friend class rtmp_chunk_stream;
	friend class rtmp_handshake;
public:
	rtmp_session(tcp::socket socket)
		: tcp_session(std::move(socket)), hs(this) {}
	~rtmp_session() { }

	void write_chunk(uint16_t csid, uint32_t time, uint32_t dtime, uint8_t type_id, uint32_t stream_id, std::shared_ptr<std::vector<uint8_t>>, bool force_f0 = false);
	void write_chunk(uint16_t csid, uint32_t time, uint32_t dtime, uint8_t type_id, uint32_t stream_id, const uint8_t* msg, int msg_size, bool force_f0 = false);
	void write_media_packet(std::shared_ptr<media_packet>);
private:
	enum
	{
		INIT,
		HANDSHAKE_C2,
		CONNECT,
		CREATE_STREAM,
		PUBLISH_PLAY,
		MEDIA_TRANSFER,
		CLOSE
	} state = INIT;

	std::deque<uint8_t> recv_buf;

	rtmp_handshake hs;
	std::unordered_map<uint16_t, std::shared_ptr<rtmp_chunk_stream>> cs_in, cs_out;

	uint32_t max_chunk_size = 128;
	uint32_t window_size = 2500000;
	uint32_t peer_bandwidth = 2500000;
	uint8_t peer_bandwidth_type = 0;
	uint32_t ack_recv = 0;
	uint32_t ack_recv_total = 0;

	bool header_read = false;
	uint16_t csid_;
	uint8_t fmt_;

	std::string app, cid, url;

	void on_connect();
	void on_write(size_t);
	void on_read(char*, size_t);
	void on_close();
	void on_error(boost::system::error_code);

	int handle();
	int handle_chunk_stream();
	int read_chunk_stream();

	


	int handle_media_message();
	int handle_control_message();
	int handle_command_message();


	void send_set_chunk_size();//1
	void send_ack();//3
public:
	void send_stream_begin(uint32_t stream_id);//4-0
	void send_stream_eof(uint32_t stream_id);//4-1
	void send_stream_dry(uint32_t stream_id);//4-2
	void send_stream_isrecorded(uint32_t stream_id);//4-4
	void send_ping_request();//4-6
private:
	void send_window_ack();//5
	void send_set_peer_bandwidth();//6

	void send_onStatus(unsigned tid, std::string level, std::string code, std::string desc);


	virtual void on_stream_create(ClientID_t) {}
};


#endif