#ifndef VEHICLE_DETECTION_HPP
#define VEHICLE_DETECTION_HPP

#include <string>
#include <chrono>
#include <DDS/core/media/avframe_writer.hpp>
#include <DDS/core/async_handler.hpp>
#include <DDS/websocket/server.hpp>

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
    VehicleDetector(std::shared_ptr<WebsocketServer>, std::string cascade_filepath);
    ~VehicleDetector();

    void write_frame(const ClientID_t, AVFrame*);
private:
    std::shared_ptr<WebsocketServer> server;
    cv::CascadeClassifier* cascade_classifier;

    AVFrame* nf = nullptr;
    SwsContext* swsctx = nullptr;

    std::chrono::time_point<std::chrono::steady_clock> last;

    void handle(ClientFramePacked);
};

#endif