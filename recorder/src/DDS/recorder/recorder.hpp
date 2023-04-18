#ifndef MEDIA_RECORDER_HPP
#define MEDIA_RECORDER_HPP

#include <string>
#include <cstdint>
#include <DDS/core/media/encoder.hpp>
#include <DDS/core/media/avframe_writer.hpp>

class AVFormatContext;
class AVStream;
class SwsContext;
class media_recorder : media_encoder, public AVFrameWriter
{
public:
    media_recorder(std::string filename);
    ~media_recorder();

    void write_frame(const ClientID_t, AVFrame*);
private:
    AVFormatContext* oc = nullptr;
    AVStream* vs = nullptr;

    AVFrame* nf = nullptr;
    SwsContext* swsctx = nullptr;

    uint64_t ts = 0;

    void on_encoded(AVPacket*);
};

#endif // !MEDIA_RECORDER
