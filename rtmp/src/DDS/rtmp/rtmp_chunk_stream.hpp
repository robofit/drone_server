#ifndef RTMP_CHUNK_STREAM_HPP
#define RTMP_CHUNK_STREAM_HPP

#include <memory>
#include <cstdint>
#include <vector>

class rtmp_session;
class rtmp_chunk_stream
{
public:
    rtmp_chunk_stream(rtmp_session*, uint16_t csid);

    int read_header(uint8_t fmt, uint16_t csid);
    int read_payload();
    void clear();

    bool ready() const { return ready_; }
    std::shared_ptr<std::vector<uint8_t>> data() const { return data_; }

    uint8_t type() const { return type_; }
    uint32_t len() const { return len_; }
    uint32_t time() const { return time_; }
    uint32_t dtime() const { return dtime_; }
    uint32_t stream() const { return stream_id_; }
    uint32_t rem() const { return rem_; }

    uint32_t write_chunk(uint32_t time, uint32_t dtime, uint8_t type_id, uint32_t stream_id, std::shared_ptr<std::vector<uint8_t>>, bool force_f0);
    uint32_t write_chunk(uint32_t time, uint32_t dtime, uint8_t type_id, uint32_t stream_id, const uint8_t* msg, uint32_t msg_size, bool force_f0);
private:
    rtmp_session* session;

    uint32_t time_ = -1, dtime_ = -1, len_ = 0, stream_id_ = -1;
    uint8_t type_ = 0;
    bool ext_ts_ = false;

    uint8_t fmt_;
    uint16_t csid_;

    std::shared_ptr<std::vector<uint8_t>> data_;

    uint32_t rem_;
    bool ready_ = false;

    enum
    {
        HEADER,
        PAYLOAD
    } state = HEADER;


    int read_f0();
    int read_f1();
    int read_f2();
    int read_f3();
};

#endif