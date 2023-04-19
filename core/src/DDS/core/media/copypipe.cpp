#include <DDS/core/media/copypipe.hpp>

void media_copypipe::write_frame(AVFrame* f)
{
    for (auto& w : writers)
    {
        w->write_frame(cid, f);
    }
}

void media_copypipe::add_writer(std::shared_ptr<AVFrameWriter> writer)
{
    writers.push_back(writer);
}