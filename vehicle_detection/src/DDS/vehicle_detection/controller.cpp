#include <DDS/vehicle_detection/controller.hpp>
#include <DDS/core/settings.hpp>
#include <DDS/core/client_pool.hpp>
#include <DDS/core/media/manager.hpp>
#include <DDS/websocket/json.hpp>

using json = nlohmann::json;

bool VehicleDetectionController::handle_string(const ClientID_t cid, std::string const& msg)
{
    settings& sett = settings::get();
    try
    {
        json j = json::parse(msg);

        if(j["type"] == "vehicle_detection")
        {
            ClientID_t src = j["data"]["client_id"];
            ClientID_t drone_id = j["data"]["drone_stream_id"];

            if(cid == src)
            {
                if(detectors.count(drone_id) == 0)
                {
                    detectors[drone_id] = std::make_shared<VehicleDetector>(sett.dstring["vehicle_detection_filepath"]);
                    media_manager::get().pipe(drone_id)->add_writer(detectors[drone_id]);
                }

                detectors[drone_id]->add(src);
                return true;
            }
        }
    }
    catch (json::exception const& ex)
    {
        //no need to handle - just return false;
    }
    return false;
}
