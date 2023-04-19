#ifndef RTMP_HANDSHAKE_HPP
#define RTMP_HANDSHAKE_HPP

#include <cstdint>

class rtmp_session;
class rtmp_handshake
{
public:
    rtmp_handshake(rtmp_session* session);
    ~rtmp_handshake();

    int handle_c0c1();
    int handle_c2();
    int send_s0s1s2();

    static constexpr unsigned c0_size = 1;
    static constexpr unsigned c1_size = 1536;
    static constexpr unsigned c2_size = 1536;
private:
    rtmp_session* session_ = nullptr;
    uint8_t *cdigest, *sdigest;

    uint8_t c0_version;
    uint32_t c1_time, c1_zero;
    bool old = false;

    int parse_c0();
    int parse_c1();
    int parse_c2();

    uint8_t m[c1_size];
};

#endif