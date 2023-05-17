#include <DDS/vehicle_detection/controller.hpp>
#include <DDS/core/settings.hpp>
#include <DDS/core/client_pool.hpp>
#include <DDS/core/media/manager.hpp>
#include <DDS/core/flight_data/client.hpp>
#include <DDS/websocket/json.hpp>

using json = nlohmann::json;

bool VehicleDetectionController::handle_string(const ClientID_t cid, std::string const& msg)
{
    settings& sett = settings::get();
    auto& cpool = client_pool::get();
    try
    {
        json j = json::parse(msg);

        if(j["type"] == "vehicle_detection_get")
        {
            ClientID_t drone_id = cid_from_hex(j["data"]["drone_stream_id"]);
            j = {
                { "type", "vehicle_detection_get_resp" },
                { "data", bool(detectors.count(drone_id) > 0 && detectors[drone_id]->count(cid) > 0) }
            };

            if(cpool.count(cid))
                std::dynamic_pointer_cast<FlightDataClient>(cpool.client(cid))->send(j.dump());
            return true;
        }
        else if(j["type"] == "vehicle_detection_set")
        {
            ClientID_t drone_id = cid_from_hex(j["data"]["drone_stream_id"]);
            bool state = j["data"]["state"];

            if(detectors.count(drone_id) == 0)
            {
                detectors[drone_id] = std::make_shared<VehicleDetector>(sett.dstring["vehicle_detection_filepath"]);
                media_manager::get().pipe(drone_id)->add_writer(detectors[drone_id]);
            }

            if(!detectors[drone_id]->count(cid) && state)
            {
                detectors[drone_id]->add(cid);
            }
            else if(detectors[drone_id]->count(cid) && !state)
            {
                detectors[drone_id]->del(cid);
            }
            else
            {
                //ignored
            }

            j = {
                { "type", "vehicle_detection_set_resp" },
                { "data", nullptr }
            };

            if(cpool.count(cid))
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
