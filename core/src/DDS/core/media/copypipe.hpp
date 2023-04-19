#ifndef MEDIA_COPYPIPE_HPP
#define MEDIA_COPYPIPE_HPP

#include <vector>
#include <memory>
#include <DDS/core/media/avframe_writer.hpp>

class media_copypipe : public AVFrameWriter
{
public:
    media_copypipe(ClientID_t cid_)
    : cid(cid_) {}

    unsigned count() const { return writers.size(); }
    void add_writer(std::shared_ptr<AVFrameWriter>);

    void write_frame(AVFrame*);
private:
    const ClientID_t cid;
    std::vector<std::shared_ptr<AVFrameWriter>> writers;
};

#endif