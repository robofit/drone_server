#include <DDS/core/media/copypipe.hpp>

extern "C"
{
#include <libavcodec/avcodec.h>
}

media_copypipe::media_copypipe(ClientID_t cid_)
    : cid(cid_)
{
    
}

media_copypipe::~media_copypipe()
{
    av_frame_free(&header);
}

void media_copypipe::write_frame(AVFrame* f)
{
    for (auto& w : writers)
    {
        w->write_frame(cid, f);
    }
}

void media_copypipe::write_header(AVFrame* f)
{
    header = av_frame_clone(f);
    write_frame(f);
}

void media_copypipe::add_writer(std::shared_ptr<AVFrameWriter> writer)
{
    if(header)
        writer->write_frame(cid, header);
    writers.insert(writer);
}

void media_copypipe::del_writer(std::shared_ptr<AVFrameWriter> writer)
{
    writers.erase(writer);
}