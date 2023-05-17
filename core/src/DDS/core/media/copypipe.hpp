#ifndef MEDIA_COPYPIPE_HPP
#define MEDIA_COPYPIPE_HPP

#include <set>
#include <memory>
#include <DDS/core/media/avframe_writer.hpp>

class media_copypipe : public AVFrameWriter
{
public:
    media_copypipe(ClientID_t);
    ~media_copypipe();

    unsigned count() const { return writers.size(); }
    void add_writer(std::shared_ptr<AVFrameWriter>);
    void del_writer(std::shared_ptr<AVFrameWriter>);

    void write_frame(AVFrame*);
    void write_header(AVFrame*);
private:
    const ClientID_t cid;
    std::set<std::shared_ptr<AVFrameWriter>> writers;
    AVFrame* header = nullptr;
};

#endif