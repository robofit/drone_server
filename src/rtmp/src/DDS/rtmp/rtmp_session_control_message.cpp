#include <DDS/rtmp/rtmp_session.hpp>
#include <DDS/core/logger.hpp>
#include <ctime>

template <typename T>
class swap_endian
{
public:
    template <typename U>
    swap_endian(U&& u) : eswap({ std::forward<U>(u) })
    {
        for (uint8_t i = 0; i < sizeof(T) / 2; ++i)
            std::swap(eswap.u8[i], eswap.u8[sizeof(T) - i - 1]);
    }
    operator T && () { return std::move(eswap.u); }
private:
    union
    {
        T u;
        uint8_t u8[sizeof(T)];
    }eswap;
};


int rtmp_session::handle_control_message()
{
    uint8_t type = cs_in[csid_]->type();
    if (type == 1)
    {
        max_chunk_size = *reinterpret_cast<uint32_t*>(cs_in[csid_]->data()->data());
        max_chunk_size = swap_endian<uint32_t>(max_chunk_size);

        LOG(DEBUG) << "<rtmp> " << "new chunk size: " << int(max_chunk_size);
    }
    else if (type == 2)
    {
        uint32_t csid = *reinterpret_cast<uint32_t*>(cs_in[csid_]->data()->data());
        csid = swap_endian<uint32_t>(csid);

        cs_in.erase(csid);

        LOG(DEBUG) << "<rtmp> " << "abort message received for chunk stream: " << int(csid);
    }
    else if (type == 3)
    {
        uint32_t seq_num = *reinterpret_cast<uint32_t*>(cs_in[csid_]->data()->data());
        seq_num = swap_endian<uint32_t>(seq_num);
        LOG(DEBUG) << "<rtmp> " << "seq num received: " << seq_num;
    }
    else if (type == 4)
    {
        uint8_t* data = cs_in[csid_]->data()->data();

        uint16_t et = *reinterpret_cast<uint16_t*>(data);
        et = swap_endian<uint16_t>(et);

        if (et == 3)
        {
            uint32_t et_stream_id = *reinterpret_cast<uint32_t*>(data + sizeof(uint16_t));
            et_stream_id = swap_endian<uint32_t>(et_stream_id);

            uint32_t buffer_len = *reinterpret_cast<uint32_t*>(data + sizeof(uint16_t) + sizeof(uint32_t));
            buffer_len = swap_endian<uint32_t>(buffer_len);

            LOG(DEBUG) << "<rtmp> " << "client buffer len request: " << int(buffer_len) << " for stream id: " << int(et_stream_id);
        }
        else if(et == 7)
        {
            uint32_t et_timestamp = *reinterpret_cast<uint32_t*>(data + sizeof(uint16_t));
            et_timestamp = swap_endian<uint32_t>(et_timestamp);

            LOG(DEBUG) << "<rtmp> " << "client pong response timestamp: " << int(et_timestamp);
        }
        else
        {
            LOG(WARN) << "<rtmp> " << "unimplemented control type: " << int(type) << "-" << int(et);
        }
    }
    else if (type == 5)
    {
        window_size = *reinterpret_cast<uint32_t*>(cs_in[csid_]->data()->data());
        window_size = swap_endian<uint32_t>(window_size);

        LOG(DEBUG) << "<rtmp> " << "window size set: " << int(window_size);
    }
    else if (type == 6)
    {
        peer_bandwidth = *reinterpret_cast<uint32_t*>(cs_in[csid_]->data()->data());
        peer_bandwidth = swap_endian<uint32_t>(peer_bandwidth);

        peer_bandwidth_type = cs_in[csid_]->data()->at(4);

        LOG(DEBUG) << "<rtmp> " << "peer bandwidth set: " << int(peer_bandwidth) << " with type: " << int(peer_bandwidth_type);
    }
    else
    {
        LOG(WARN) << "<rtmp> " << "unimplemented control type: " << int(type);
    }

    cs_in[csid_]->clear();
    return 0;
}


void rtmp_session::send_set_chunk_size()
{
    uint32_t msg = max_chunk_size;
    msg = swap_endian<uint32_t>(msg);

    write_chunk(2, 0, 0, 1, 0, reinterpret_cast<uint8_t*>(&msg), sizeof(msg), true);
}

void rtmp_session::send_ack()
{
    uint32_t msg = ack_recv_total;
    msg = swap_endian<uint32_t>(msg);

    write_chunk(2, 0, 0, 3, 0, reinterpret_cast<uint8_t*>(&msg), sizeof(msg), true);
}

void rtmp_session::send_stream_begin(uint32_t stream_id)
{
    uint8_t msg[6];
    msg[0] = 0;
    msg[1] = 0;

    msg[2] = (stream_id >> 24) & 0xFF;
    msg[3] = (stream_id >> 16) & 0xFF;
    msg[4] = (stream_id >>  8) & 0xFF;
    msg[5] = (stream_id >>  0) & 0xFF;

    write_chunk(2, 0, 0, 4, 0, msg, sizeof(msg), true);
}

void rtmp_session::send_stream_eof(uint32_t stream_id)
{
    uint8_t msg[6];
    msg[0] = 0;
    msg[1] = 1;

    msg[2] = (stream_id >> 24) & 0xFF;
    msg[3] = (stream_id >> 16) & 0xFF;
    msg[4] = (stream_id >>  8) & 0xFF;
    msg[5] = (stream_id >>  0) & 0xFF;

    write_chunk(2, 0, 0, 4, 0, msg, sizeof(msg), true);
}

void rtmp_session::send_stream_dry(uint32_t stream_id)
{
    uint8_t msg[6];
    msg[0] = 0;
    msg[1] = 2;

    msg[2] = (stream_id >> 24) & 0xFF;
    msg[3] = (stream_id >> 16) & 0xFF;
    msg[4] = (stream_id >>  8) & 0xFF;
    msg[5] = (stream_id >>  0) & 0xFF;

    write_chunk(2, 0, 0, 4, 0, msg, sizeof(msg), true);
}

void rtmp_session::send_stream_isrecorded(uint32_t stream_id)
{
    uint8_t msg[6];
    msg[0] = 0;
    msg[1] = 4;

    msg[2] = (stream_id >> 24) & 0xFF;
    msg[3] = (stream_id >> 16) & 0xFF;
    msg[4] = (stream_id >>  8) & 0xFF;
    msg[5] = (stream_id >>  0) & 0xFF;

    write_chunk(2, 0, 0, 4, 0, msg, sizeof(msg), true);
}

void rtmp_session::send_ping_request()
{
    uint8_t msg[6];
    msg[0] = 0;
    msg[1] = 6;

    uint32_t ct = std::time(0);

    msg[2] = (ct >> 24) & 0xFF;
    msg[3] = (ct >> 16) & 0xFF;
    msg[4] = (ct >>  8) & 0xFF;
    msg[5] = (ct >>  0) & 0xFF;

    write_chunk(2, 0, 0, 4, 0, msg, sizeof(msg), true);
}

void rtmp_session::send_window_ack()
{
    uint32_t msg = window_size;
    msg = swap_endian<uint32_t>(msg);

    write_chunk(2, 0, 0, 5, 0, reinterpret_cast<uint8_t*>(&msg), sizeof(msg), true);
}

void rtmp_session::send_set_peer_bandwidth()
{
    uint8_t msg[5];
    msg[0] = (peer_bandwidth >> 24) & 0xFF;
    msg[1] = (peer_bandwidth >> 16) & 0xFF;
    msg[2] = (peer_bandwidth >>  8) & 0xFF;
    msg[3] = (peer_bandwidth >>  0) & 0xFF;

    msg[4] = peer_bandwidth_type;

    write_chunk(2, 0, 0, 6, 0, msg, sizeof(msg), true);
}