#include <DDS/rtmp/rtmp_chunk_stream.hpp>
#include <DDS/rtmp/rtmp_session.hpp>
#include <DDS/core/logger.hpp>

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


struct ChunkBasicHeader1
{
    uint8_t csid : 6;
    uint8_t fmt : 2;
};

struct ChunkMessageHeader0
{
    uint8_t time[3];
    uint8_t len[3];
    uint8_t type;
    uint8_t stream[4];
};

struct ChunkMessageHeader1
{
    uint8_t dtime[3];
    uint8_t len[3];
    uint8_t type;
};

struct ChunkMessageHeader2
{
    uint8_t dtime[3];
};

template<class T>
inline
T copy_pop_front(std::deque<T>& dq)
{
    T f = dq.front();
    dq.pop_front();
    return f;
}


rtmp_chunk_stream::rtmp_chunk_stream(rtmp_session* s, uint16_t csid)
: session(s), csid_(csid)
{
    data_ = std::make_shared<std::vector<uint8_t>>();
}

int rtmp_chunk_stream::read_f0()
{
    auto& buf = session->recv_buf;

    if (buf.size() < sizeof(ChunkMessageHeader0))
        return 1;

    auto dit = buf.cbegin();

    ChunkMessageHeader0 mh;
    std::copy(dit, dit + sizeof(ChunkMessageHeader0), reinterpret_cast<uint8_t*>(&mh));
    dit += sizeof(ChunkMessageHeader0);

    uint32_t ts = (mh.time[0] << 16) | (mh.time[1] << 8) | mh.time[2];
    if (ts == 0xffffff)
    {
        ext_ts_ = true;
        if(buf.size() < sizeof(ChunkMessageHeader0) + 4)
            return 1;
    }

    buf.erase(buf.cbegin(), dit);
    
    if (ts == 0xffffff)
    {
        ts = 0;
        ts |= copy_pop_front(buf);
        ts |= (uint32_t(copy_pop_front(buf)) << 8);
        ts |= (uint32_t(copy_pop_front(buf)) << 16);
        ts |= (uint32_t(copy_pop_front(buf)) << 24);
    }

    time_ = ts;
    dtime_ = 0;

    len_ = (mh.len[0] << 16) | (mh.len[1] << 8) | mh.len[2];
    type_ = mh.type;
    stream_id_ = (mh.stream[3] << 24) | (mh.stream[2] << 16) | (mh.stream[1] << 8) | mh.stream[0];
    
    rem_ = len_;

    return 0;
}

int rtmp_chunk_stream::read_f1()
{
    auto& buf = session->recv_buf;

    if (buf.size() < sizeof(ChunkMessageHeader1))
        return 1;

    auto dit = buf.cbegin();

    ChunkMessageHeader1 mh;
    std::copy(dit, dit + sizeof(ChunkMessageHeader1), reinterpret_cast<uint8_t*>(&mh));
    dit += sizeof(ChunkMessageHeader1);

    uint32_t ts = (mh.dtime[0] << 16) | (mh.dtime[1] << 8) | mh.dtime[2];
    if (ts == 0xffffff)
    {
        ext_ts_ = true;
        if (buf.size() < sizeof(ChunkMessageHeader1) + 4)
            return 1;
    }

    buf.erase(buf.cbegin(), dit);
    
    if (ts == 0xffffff)
    {
        ts = 0;
        ts |= copy_pop_front(buf);
        ts |= (uint32_t(copy_pop_front(buf)) << 8);
        ts |= (uint32_t(copy_pop_front(buf)) << 16);
        ts |= (uint32_t(copy_pop_front(buf)) << 24);
    }

    time_ += ts;
    dtime_ = ts;

    len_ = (mh.len[0] << 16) | (mh.len[1] << 8) | mh.len[2];
    type_ = mh.type;

    rem_ = len_;

    return 0;
}

int rtmp_chunk_stream::read_f2()
{
    auto& buf = session->recv_buf;

    if (buf.size() < sizeof(ChunkMessageHeader2))
        return 1;

    auto dit = buf.cbegin();

    ChunkMessageHeader2 mh;
    std::copy(dit, dit + sizeof(ChunkMessageHeader2), reinterpret_cast<uint8_t*>(&mh));
    dit += sizeof(ChunkMessageHeader2);

    uint32_t ts = (mh.dtime[0] << 16) | (mh.dtime[1] << 8) | mh.dtime[2];
    if (ts == 0xffffff)
    {
        ext_ts_ = true;
        if (buf.size() < sizeof(ChunkMessageHeader2) + 4)
            return 1;
    }

    buf.erase(buf.cbegin(), dit);

    if (ts == 0xffffff)
    {
        ts = 0;
        ts |= copy_pop_front(buf);
        ts |= (uint32_t(copy_pop_front(buf)) << 8);
        ts |= (uint32_t(copy_pop_front(buf)) << 16);
        ts |= (uint32_t(copy_pop_front(buf)) << 24);
    }

    time_ += ts;
    dtime_ = ts;

    rem_ = len_;

    return 0;
}

int rtmp_chunk_stream::read_f3()
{
    auto& buf = session->recv_buf;

    uint8_t* data;
    if (rem_ == 0)
    {
        if (fmt_ == 0)
        {
            if (ext_ts_)
            {
                if (buf.size() < 4)
                    return 1;

                time_ = 0;
                time_ |= copy_pop_front(buf);
                time_ |= (uint32_t(copy_pop_front(buf)) << 8);
                time_ |= (uint32_t(copy_pop_front(buf)) << 16);
                time_ |= (uint32_t(copy_pop_front(buf)) << 24);
            }
        }
        else if (fmt_ == 1 || fmt_ == 2)
        {
            uint32_t dts = 0;
            if (ext_ts_)
            {
                if (buf.size() < 4)
                    return 1;

                dts = 0;
                dts |= copy_pop_front(buf);
                dts |= (uint32_t(copy_pop_front(buf)) << 8);
                dts |= (uint32_t(copy_pop_front(buf)) << 16);
                dts |= (uint32_t(copy_pop_front(buf)) << 24);
            }
            else
            {
                dts = dtime_;
            }
            time_ += dts;
        }
        data_->clear();
        rem_ = len_;
        ready_ = false;
    }
    else
    {
        if (ext_ts_)
        {
            if (buf.size() < 4)
                return 1;

            uint32_t ts = (uint32_t(buf[3]) << 24) | (uint32_t(buf[2]) << 16) | (uint32_t(buf[1]) << 8) | uint32_t(buf[0]);
            if (ts == time_)
                buf.erase(buf.cbegin(), buf.cbegin() + 4);
        }
    }

    return 0;
}

int rtmp_chunk_stream::read_header(uint8_t fmt, uint16_t csid)
{
    if(state != HEADER)
        return 0;

    int ret;
    if (fmt == 0)
        ret = read_f0();
    else if (fmt == 1)
        ret = read_f1();
    else if (fmt == 2)
        ret = read_f2();
    else if (fmt == 3)
        ret = read_f3();
    else
        ret = -1;

    fmt_ = fmt;

    if (ret == 0)
        state = PAYLOAD;

    return ret;
}

int rtmp_chunk_stream::read_payload()
{
    if (state != PAYLOAD)
        return 0;
    

    auto& buf = session->recv_buf;
    uint32_t req = (rem_ > session->max_chunk_size) ? session->max_chunk_size : rem_;

    if (buf.size() < req)
        return 1;

    data_->insert(data_->cend(), buf.cbegin(), buf.cbegin() + req);

    buf.erase(buf.begin(), buf.cbegin() + req);

    rem_ -= req;
    if (rem_ == 0)
    {
        ready_ = true;
    }

    state = HEADER;
    return 0;
}

void rtmp_chunk_stream::clear()
{
    state = HEADER;
    ready_ = false;
    data_->clear();
}

uint32_t rtmp_chunk_stream::write_chunk(uint32_t time, uint32_t dtime, uint8_t type_id, uint32_t stream_id, std::shared_ptr<std::vector<uint8_t>> fb, bool force_f0)
{
    return write_chunk(time, dtime, type_id, stream_id, fb->data(), fb->size(), force_f0);
}

uint32_t rtmp_chunk_stream::write_chunk(uint32_t time, uint32_t dtime, uint8_t type_id, uint32_t stream_id, const uint8_t* msg, uint32_t msg_size, bool force_f0)
{
    uint32_t data_len = 0;

    int size = sizeof(ChunkBasicHeader1);
    ChunkBasicHeader1 bh{};

    if (dtime_ == dtime && len_ == msg_size && type_ == type_id && stream_id_ == stream_id && !force_f0)//type 3
    {
        bh.fmt = 3;
        bh.csid = csid_;
        data_->insert(data_->cend(), reinterpret_cast<uint8_t*>(&bh), reinterpret_cast<uint8_t*>(&bh) + size);

        data_len += sizeof(ChunkBasicHeader1);
    }
    else if (len_ == msg_size && type_ == type_id && stream_id_ == stream_id && !force_f0)//type 2
    {
        bh.fmt = 2;
        bh.csid = csid_;
        data_->insert(data_->cend(), reinterpret_cast<uint8_t*>(&bh), reinterpret_cast<uint8_t*>(&bh) + size);

        data_->push_back(dtime >> 16);
        data_->push_back(dtime >> 8);
        data_->push_back(dtime);

        dtime_ = dtime;

        data_len += sizeof(ChunkBasicHeader1) + sizeof(ChunkMessageHeader2);
    }
    else if (stream_id_ == stream_id && !force_f0)//type 1
    {
        bh.fmt = 1;
        bh.csid = csid_;
        data_->insert(data_->cend(), reinterpret_cast<uint8_t*>(&bh), reinterpret_cast<uint8_t*>(&bh) + size);

        data_->push_back(dtime >> 16);
        data_->push_back(dtime >> 8);
        data_->push_back(dtime);

        data_->push_back(msg_size >> 16);
        data_->push_back(msg_size >> 8);
        data_->push_back(msg_size);

        data_->push_back(type_id);

        dtime_ = dtime;
        len_ = msg_size;
        type_ = type_id;

        data_len += sizeof(ChunkBasicHeader1) + sizeof(ChunkMessageHeader1);
    }
    else//type 0
    {
        bh.fmt = 0;
        bh.csid = csid_;
        data_->insert(data_->cend(), reinterpret_cast<uint8_t*>(&bh), reinterpret_cast<uint8_t*>(&bh) + size);

        data_->push_back(time >> 16);
        data_->push_back(time >> 8);
        data_->push_back(time);

        data_->push_back(msg_size >> 16);
        data_->push_back(msg_size >> 8);
        data_->push_back(msg_size);

        data_->push_back(type_id);

        data_->push_back(stream_id);
        data_->push_back(stream_id >> 8);
        data_->push_back(stream_id >> 16);
        data_->push_back(stream_id >> 24);

        time_ = time;
        len_ = msg_size;
        type_ = type_id;
        stream_id_ = stream_id;

        data_len += sizeof(ChunkBasicHeader1) + sizeof(ChunkMessageHeader0);
    }

    if (msg_size > session->max_chunk_size)
    {
        data_->insert(data_->end(), msg, msg + session->max_chunk_size);
        data_len += session->max_chunk_size;

        session->async_write(data_);
        data_->clear();

        msg_size -= session->max_chunk_size;
        msg += session->max_chunk_size;

        bh.fmt = 3;
        bh.csid = csid_;
        uint32_t len;
        while (msg_size > 0)
        {
            data_->insert(data_->cend(), reinterpret_cast<uint8_t*>(&bh), reinterpret_cast<uint8_t*>(&bh) + size);
            data_len += sizeof(ChunkBasicHeader1);

            len = msg_size > session->max_chunk_size ? session->max_chunk_size : msg_size;
            data_->insert(data_->end(), msg, msg + len);
            data_len += len;

            session->async_write(data_);
            data_->clear();

            msg_size -= len;
            msg += len;
        }
    }
    else
    {
        data_->insert(data_->end(), msg, msg + msg_size);
        data_len += msg_size;

        session->async_write(data_);
        data_->clear();
    }
    
    return data_len;
}
