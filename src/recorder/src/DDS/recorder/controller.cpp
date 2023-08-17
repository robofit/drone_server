#include <DDS/recorder/controller.hpp>
#include <DDS/core/flight_data/client.hpp>
#include <DDS/core/media/manager.hpp>
#include <DDS/core/client_pool.hpp>
#include <DDS/websocket/json.hpp>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>

using json = nlohmann::json;

bool RecorderController::handle_string(const ClientID_t cid, std::string const& msg)
{
    auto& cpool = client_pool::get();
    try
    {
        json j = json::parse(msg);

        if(j["type"] == "media_record_get")
        {
            ClientID_t drone_id = cid_from_hex(j["data"]["drone_stream_id"]);
            j = {
                { "type", "media_record_get_resp" },
                { "data", bool(recorders.count(drone_id)) }
            };

            if(cpool.count(cid))
                std::dynamic_pointer_cast<FlightDataClient>(cpool.client(cid))->send(j.dump());
            return true;
        }
        else if(j["type"] == "media_record_set")
        {
            ClientID_t drone_id = cid_from_hex(j["data"]["drone_stream_id"]);
            bool state = j["data"]["state"];

            if(recorders.count(drone_id) == 0 && state)
            {
                auto curr_time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

                std::ostringstream oss;
                oss << std::put_time(std::localtime(&curr_time), "%Y%m%d-%H%M%S");
                oss << "-";
                oss << cid_to_hex(drone_id);
                oss << ".mp4";

                recorders[drone_id] = std::make_shared<media_recorder>(oss.str());
                media_manager::get().pipe(drone_id)->add_writer(recorders[drone_id]);
            }
            else if(recorders.count(drone_id) > 0 && !state)
            {
                media_manager::get().pipe(drone_id)->del_writer(recorders[drone_id]);
                recorders.erase(drone_id);
            }
            else
            {
                //ignored
            }
            
            j = {
                { "type", "media_record_set_resp" },
                { "data", nullptr }
            };

            std::dynamic_pointer_cast<FlightDataClient>(cpool.client(cid))->send(j.dump());
            return true;
        }
    }
    catch (json::exception const& ex)
    {
        //no need to handle - just return false;
    }
    return false;
}