#ifndef VEHICLE_DETECTION_HPP
#define VEHICLE_DETECTION_HPP

#include <set>
#include <string>
#include <chrono>
#include <DDS/core/media/avframe_writer.hpp>
#include <DDS/core/async_handler.hpp>

struct ClientFramePacked
{
    ClientID_t cid;
    AVFrame* frame;
};

namespace cv
{
    class CascadeClassifier;
}
class SwsContext;
class VehicleDetector : public AVFrameWriter, async_handler<ClientFramePacked>
{
public:
    VehicleDetector(std::string cascade_filepath);
    ~VehicleDetector();

    void write_frame(const ClientID_t, AVFrame*);

    void add(ClientID_t);
    void del(ClientID_t);
private:
    cv::CascadeClassifier* cascade_classifier;
    std::set<ClientID_t> dsts;
    std::mutex dstsm;

    AVFrame* nf = nullptr;
    SwsContext* swsctx = nullptr;

    std::chrono::time_point<std::chrono::steady_clock> last;

    void handle(ClientFramePacked);
};

#endif