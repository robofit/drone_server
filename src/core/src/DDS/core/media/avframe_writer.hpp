#ifndef AVFRAME_WRITER_HPP
#define AVFRAME_WRITER_HPP

#include <DDS/core/client.hpp>

class AVFrame;
class AVFrameWriter
{
public:
    virtual void write_frame(const ClientID_t, AVFrame*) {}
};

#endif