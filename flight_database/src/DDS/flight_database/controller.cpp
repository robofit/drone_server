#include <DDS/flight_database/controller.hpp>
#include <DDS/websocket/json.hpp>
#include <DDS/core/client_pool.hpp>
#include <DDS/core/flight_data/client.hpp>
#include <DDS/core/flight_data/manager.hpp>
#include <exception>
#include <DDS/core/logger.hpp>

using json = nlohmann::json;

bool FlightDatabaseController::handle_string(const ClientID_t cid, std::string const& msg)
{
    auto& cpool = client_pool::get();
    flight_data_manager& fdman = flight_data_manager::get();
    try
    {
        json j = json::parse(msg);

        if(j["type"] == "flight_data_save_get")
        {
            j = {
                { "type", "flight_data_save_get_resp" },
                { "data", bool(fd_ptr) }
            };

            if(cpool.count(cid))
                std::dynamic_pointer_cast<FlightDataClient>(cpool.client(cid))->send(j.dump());
            return true;
        }
        else if(j["type"] == "flight_data_save_set")
        {
            if(!fd_ptr && j["data"])
            {
                fd_ptr = std::make_shared<FlightDatabase>();
                fdman.add(fd_ptr);
            }
            else if(fd_ptr && !j["data"])
            {
                fdman.del(fd_ptr);
                fd_ptr.reset();
            }
            else
                return false;
            

            j = {
                { "type", "flight_data_save_set_resp" },
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
    catch (std::exception const& ex)
    {
        LOG(ERROR) << ex.what();
        return true;
    }
    return false;
}