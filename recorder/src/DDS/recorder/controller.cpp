#include <DDS/recorder/controller.hpp>
#include <DDS/recorder/recorder.hpp>
#include <DDS/core/media/manager.hpp>
#include <DDS/websocket/json.hpp>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>

using json = nlohmann::json;

bool RecorderController::handle_string(const ClientID_t, std::string const& msg)
{
    try
    {
        json j = json::parse(msg);

        if(j["type"] == "media_record")
        {
            ClientID_t drone_id = j["data"]["drone_stream_id"];

            auto curr_time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

            std::ostringstream oss;
            oss << std::put_time(std::localtime(&curr_time), "%Y.%m.%d-%T");
            oss << "-";
            oss << cid_to_hex(drone_id);
            oss << ".mp4";

            media_manager::get().pipe(drone_id)->add_writer(std::make_shared<media_recorder>(oss.str()));
            return true;
        }
    }
    catch (json::exception const& ex)
    {
        //no need to handle - just return false;
    }
    return false;
}