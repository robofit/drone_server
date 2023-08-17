#include <DDS/rtmp/rtmp_handshake.hpp>
#include <DDS/rtmp/rtmp_session.hpp>
#include <DDS/core/logger.hpp>

#include <openssl/hmac.h>
#include <openssl/sha.h>
#include <openssl/rand.h>

#define RTMP_HANDSHAKE_KEYLEN SHA256_DIGEST_LENGTH

static constexpr
std::array<unsigned char, 68> RTMP_SERVER_KEY
{
    'G', 'e', 'n', 'u', 'i', 'n', 'e', ' ', 'A', 'd', 'o', 'b', 'e', ' ',
    'F', 'l', 'a', 's', 'h', ' ', 'M', 'e', 'd', 'i', 'a', ' ',
    'S', 'e', 'r', 'v', 'e', 'r', ' ',
    '0', '0', '1',

    0xF0, 0xEE, 0xC2, 0x4A, 0x80, 0x68, 0xBE, 0xE8, 0x2E, 0x00, 0xD0, 0xD1,
    0x02, 0x9E, 0x7E, 0x57, 0x6E, 0xEC, 0x5D, 0x2D, 0x29, 0x80, 0x6F, 0xAB,
    0x93, 0xB8, 0xE6, 0x36, 0xCF, 0xEB, 0x31, 0xAE
};

static constexpr
std::array<unsigned char, 62> RTMP_CLIENT_KEY
{
    'G', 'e', 'n', 'u', 'i', 'n', 'e', ' ', 'A', 'd', 'o', 'b', 'e', ' ',
    'F', 'l', 'a', 's', 'h', ' ', 'P', 'l', 'a', 'y', 'e', 'r', ' ',
    '0', '0', '1',

    0xF0, 0xEE, 0xC2, 0x4A, 0x80, 0x68, 0xBE, 0xE8, 0x2E, 0x00, 0xD0, 0xD1,
    0x02, 0x9E, 0x7E, 0x57, 0x6E, 0xEC, 0x5D, 0x2D, 0x29, 0x80, 0x6F, 0xAB,
    0x93, 0xB8, 0xE6, 0x36, 0xCF, 0xEB, 0x31, 0xAE
};

static constexpr
std::array<unsigned char, 36> RTMP_PARTIAL_SERVER_KEY
{
    'G', 'e', 'n', 'u', 'i', 'n', 'e', ' ', 'A', 'd', 'o', 'b', 'e', ' ',
    'F', 'l', 'a', 's', 'h', ' ', 'M', 'e', 'd', 'i', 'a', ' ',
    'S', 'e', 'r', 'v', 'e', 'r', ' ',
    '0', '0', '1'
};

static constexpr
std::array<unsigned char, 30> RTMP_PARTIAL_CLIENT_KEY
{
    'G', 'e', 'n', 'u', 'i', 'n', 'e', ' ', 'A', 'd', 'o', 'b', 'e', ' ',
    'F', 'l', 'a', 's', 'h', ' ', 'P', 'l', 'a', 'y', 'e', 'r', ' ',
    '0', '0', '1'
};

inline
int hmac_sha256(std::vector<unsigned char> key, const unsigned char* data_, const int data_size_, const unsigned char* skip, unsigned char* digest)
{
    const unsigned char* last = data_ + data_size_;
    std::vector<uint8_t> data;
    if(skip && data_ <= skip && skip <= last)
    {
        if (skip != data_)
        {
            data.insert(data.end(), data_, data_ + (skip - data_));
        }
        if (last != skip + RTMP_HANDSHAKE_KEYLEN)
        {
            data.insert(data.end(), skip + RTMP_HANDSHAKE_KEYLEN, last);
        }
    }
    else
    {
        data.insert(data.end(), data_, data_ + data_size_);
    }


    unsigned int digest_size = 0;
    
    uint8_t* temp_key = (uint8_t*)key.data();
    uint8_t* temp_digest = (uint8_t*)digest;

#if OPENSSL_VERSION_NUMBER < 0x1010000fL
    HMAC_CTX ctx;

    HMAC_CTX_init(&ctx);

    if (HMAC_Init_ex(&ctx, temp_key, key_size, EVP_sha256(), NULL) < 0) {
        return -1;
    }

    if (HMAC_Update(&ctx, data.data(), data.size()) < 0) {
        return -1;
    }
    
    if (HMAC_Final(&ctx, temp_digest, &digest_size) < 0) {
        return -1;
    }

    HMAC_CTX_cleanup(&ctx);
#elif OPENSSL_VERSION_NUMBER > 0x3L
    temp_digest = HMAC(EVP_sha256(), temp_key, key.size(), data.data(), data.size(), temp_digest, &digest_size);
    if(temp_digest == NULL)
        return -1;
#else
    HMAC_CTX *ctx = HMAC_CTX_new();
    if (ctx == nullptr) {
        log_errorf("hmac new error...");
        return -1;
    }

    if (HMAC_Init_ex(ctx, temp_key, key_size, EVP_sha256(), NULL) < 0) {
        HMAC_CTX_free(ctx);
        return -1;
    }

    if (HMAC_Update(ctx, data.data(), data.size()) < 0) {
        HMAC_CTX_free(ctx);
        return -1;
    }
    
    if (HMAC_Final(ctx, temp_digest, &digest_size) < 0) {
        HMAC_CTX_free(ctx);
        return -1;
    }
    HMAC_CTX_free(ctx);
#endif
    if (digest_size != RTMP_HANDSHAKE_KEYLEN) {
        return -1;
    }
    
    return 0;
}

static
int find_digest(unsigned char *b, unsigned data_size, std::vector<unsigned char>& key, unsigned base)
{
    unsigned char digest[RTMP_HANDSHAKE_KEYLEN];
    unsigned char* p;

    unsigned offs = 0;
    for (unsigned n = 0; n < 4; ++n) {
        offs += b[base + n];
    }
    offs = (offs % 728) + base + 4;
    p = b + offs;

    if(hmac_sha256(key, b, data_size, p, digest) != 0)
        return -1;

    if(memcmp(digest, p, RTMP_HANDSHAKE_KEYLEN) != 0)
        return -1;

    return offs;
}

static
int write_digest(unsigned char* b, unsigned data_size, std::vector<unsigned char>& key, unsigned base)
{
    unsigned char* p;

    unsigned offs = 0;
    for (unsigned n = 8; n < 12; ++n) {
        offs += b[base + n];
    }
    offs = (offs % 728) + base + 12;
    p = b + offs;

    if (hmac_sha256(key, b, data_size, p, p) != 0)
        return -1;

    return offs;
}

rtmp_handshake::rtmp_handshake(rtmp_session* session)
: session_(session)
{
    cdigest = new uint8_t[RTMP_HANDSHAKE_KEYLEN];
    sdigest = new uint8_t[RTMP_HANDSHAKE_KEYLEN];
}

rtmp_handshake::~rtmp_handshake()
{
    delete[] sdigest;
    delete[] cdigest;
}

inline
uint32_t read_4bytes(uint8_t* p)
{
    uint32_t tmp = 0;
    tmp |= *p++ <<  0;
    tmp |= *p++ <<  8;
    tmp |= *p++ << 16;
    tmp |= *p++ << 24;
    return tmp;
}

inline
void write_4bytes(uint8_t* p, uint32_t d)
{
    p[0] = (d >>  0) & 0xFF;
    p[1] = (d >>  8) & 0xFF;
    p[2] = (d >> 16) & 0xFF;
    p[3] = (d >> 24) & 0xFF;
}

int rtmp_handshake::parse_c0()
{
    c0_version = session_->recv_buf.front();
    session_->recv_buf.pop_front();

    if(c0_version != 3)
    {
        LOG(INFO) << "<rtmp> " << "handshake version:" << c0_version;
        return -1;
    }
    return 0;
}

int rtmp_handshake::parse_c1()
{
    std::copy(session_->recv_buf.cbegin(), session_->recv_buf.cbegin() + c1_size, m);
    session_->recv_buf.erase(session_->recv_buf.cbegin(), session_->recv_buf.cbegin() + c1_size);

    c1_time = read_4bytes(m);
    c1_zero = read_4bytes(m+4);

    LOG(INFO) << "<rtmp> " << "handshake peer version=" << int(m[7]) << '.' << int(m[6]) << '.' << int(m[5]) << '.' << int(m[4]) << " epoch=" << c1_time;

    if (c1_zero == 0)
    {
        old = true;
        return 0;
    }

    std::vector<unsigned char> peer_key(RTMP_PARTIAL_CLIENT_KEY.cbegin(), RTMP_PARTIAL_CLIENT_KEY.cend());

    int offs = find_digest(m, c1_size, peer_key, 772);
    if(offs == -1)
        offs = find_digest(m, c1_size, peer_key, 8);

    if(offs == -1)
    {
        LOG(WARN) << "<rtmp> " << "handshake digest not found -> ignoring";
        old = true;
        return 0;
    }

    std::vector<unsigned char> server_key(RTMP_SERVER_KEY.cbegin(), RTMP_SERVER_KEY.cend());
    if(hmac_sha256(server_key, m + offs, RTMP_HANDSHAKE_KEYLEN, nullptr, cdigest) != 0)
    {
        LOG(ERROR) << "<rtmp> " << "handshake hmac error";
        return -1;
    }

    return 0;
}

int rtmp_handshake::parse_c2()
{
    if (old)
    {
        session_->recv_buf.erase(session_->recv_buf.cbegin(), session_->recv_buf.cbegin() + c2_size);
        return 0;
    }

    std::copy(session_->recv_buf.cbegin(), session_->recv_buf.cbegin() + c2_size, m);
    session_->recv_buf.erase(session_->recv_buf.cbegin(), session_->recv_buf.cbegin() + c2_size);

    std::vector<unsigned char> digest_key(sdigest, sdigest + RTMP_HANDSHAKE_KEYLEN);

    int offs = find_digest(m, c2_size, digest_key, 772);
    if(offs == -1)
        offs = find_digest(m, c2_size, digest_key, 8);

    if(offs == -1)
        offs = c2_size - RTMP_HANDSHAKE_KEYLEN;

    unsigned char digest[RTMP_HANDSHAKE_KEYLEN];
    if (hmac_sha256(digest_key, m, offs, nullptr, digest) != 0)
    {
        LOG(ERROR) << "<rtmp> " << "handshake hmac error";
        return -1;
    }

    if (memcmp(digest, m + offs, RTMP_HANDSHAKE_KEYLEN) != 0)
    {
        LOG(INFO) << "<rtmp> " << "handshake invalid digest - ignoring";
        return 0;
    }

    return 0;
}

int rtmp_handshake::handle_c0c1()
{
    if (session_->recv_buf.size() < c0_size + c1_size)
        return 1;

    if(parse_c0() != 0)
        return -1;

    if(parse_c1() != 0)
        return -1;

    return 0;
}

int rtmp_handshake::handle_c2()
{
    if (session_->recv_buf.size() < c2_size)
        return 1;

    if (parse_c2() != 0)
        return -1;

    return 0;
}


int rtmp_handshake::send_s0s1s2()
{
    char s0s1s2[c0_size + c1_size + c2_size];
    uint8_t* p = (uint8_t*)s0s1s2;

    //init with random bytes
    if(RAND_bytes(p, sizeof(s0s1s2)) != 1)
    {
        LOG(ERROR) << "<rtmp> " << "handshake openssl rand error";
        return -1;
    }

    //s0
    p[0] = 0x03;
    p++;

    //s1
    if (old)
    {
        write_4bytes(p, 0);
        write_4bytes(p + 4, 0);
    }
    else
    {
        write_4bytes(p, c1_time);
        p[7] = 13;
        p[6] = 12;
        p[5] = 0;
        p[4] = 10;

        std::vector<unsigned char> ps_key(RTMP_PARTIAL_SERVER_KEY.cbegin(), RTMP_PARTIAL_SERVER_KEY.cend());
        int offs = write_digest(p, c1_size, ps_key, 0);
        if (offs == -1)
        {
            LOG(ERROR) << "<rtmp> " << "handshake write_digest error";
            return -1;
        }

        std::vector<unsigned char> client_key(RTMP_CLIENT_KEY.cbegin(), RTMP_CLIENT_KEY.cend());
        if (hmac_sha256(client_key, p + offs, RTMP_HANDSHAKE_KEYLEN, nullptr, sdigest) != 0)
        {
            LOG(ERROR) << "<rtmp> " << "handshake hmac error";
            return -1;
        }
    }
    p += c1_size;

    //s2
    if (old)
    {
        write_4bytes(m, c1_time);
        write_4bytes(m + 4, 0);
        std::copy(m, m + sizeof(m), p);
    }
    else
    {
        unsigned skip = c2_size - RTMP_HANDSHAKE_KEYLEN;

        std::vector<unsigned char> digest_key(cdigest, cdigest + RTMP_HANDSHAKE_KEYLEN);
        if(hmac_sha256(digest_key, p, skip, 0, p + skip) != 0)
        {
            LOG(ERROR) << "<rtmp> " << "handshake hmac error";
            return -1;
        }
    }

    session_->async_write(s0s1s2, sizeof(s0s1s2));
    return 1;
}