#include <DDS/websocket/client.hpp>
#include <DDS/websocket/server.hpp>
#include <DDS/websocket/json.hpp>

#include <DDS/core/logger.hpp>
#include <DDS/core/settings.hpp>
#include <DDS/core/client_pool.hpp>
#include <DDS/core/flight_data/server.hpp>
#include <DDS/core/flight_data/manager.hpp>

#include <websocketpp/close.hpp>

using json = nlohmann::json;

WebsocketClient::WebsocketClient(std::shared_ptr<FlightDataServer> s)
    : FlightDataClient(s)
{

}

WebsocketClient::~WebsocketClient()
{
    client_pool::get().del(shared_from_this());
}

void WebsocketClient::recv(std::string msg)
{
    add_job(msg);
}

void WebsocketClient::send(std::string const& msg)
{
    std::dynamic_pointer_cast<WebsocketServer>(server)->send(shared_from_this(), msg);
}

void WebsocketClient::handle(std::string msg)
{
    try
    {
        json j = json::parse(msg);

        if(j["type"] == "hello" && !handshake_done)
        {
            type = static_cast<Client::Type>(j["data"]["ctype"]);
            if(type == Client::Type::DRONE)
            {
                drone_name = j["data"]["drone_name"];
                serial = j["data"]["serial"];
            }
            else if(type == Client::Type::OPERATOR)
            {
            }

            client_pool::get().add(shared_from_this());
            handshake_done = true;

            on_hello();
        }
        else if(j["type"] == "data_broadcast" && handshake_done)
        {
            on_data(j.dump());
        }
        else if(j["type"] == "drone_list" && handshake_done)
        {
            on_drone_list();
        }
        else if(flight_data_manager::get().handle(msg))
        {
            
        }
        else
        {
            LOG(ERROR) << "<websocket> " << "unsupported request";
            std::dynamic_pointer_cast<WebsocketServer>(server)->kick(shared_from_this(), websocketpp::close::status::invalid_payload, "unsupported request");
        }
    }
    catch (json::exception const& ex)
    {
        LOG(ERROR) << "<websocket> " << "parsing error";
        std::dynamic_pointer_cast<WebsocketServer>(server)->kick(shared_from_this(), websocketpp::close::status::invalid_payload, "parsing error");
    }
}

void WebsocketClient::on_hello()
{
    json j =
    {
        {"type", "hello_resp"},
        {"data",
            {
                {"client_id", cid_to_hex(id)}
            }
        }
    };

    if(type == Client::Type::OPERATOR)
    {
        j["data"]["rtmp_port"] = settings::get().dint.count("rtmp_port") > 0 ? settings::get().dint["rtmp_port"] : 0;
    }

    std::dynamic_pointer_cast<WebsocketServer>(server)->send(shared_from_this(), j.dump());
}

void WebsocketClient::on_data(std::string const& data)
{
    for (auto c : client_pool::get().clients())
    {
        auto fdc = std::dynamic_pointer_cast<FlightDataClient>(c);
        if(fdc)
            fdc->send(data);
    }
}

void to_json(json& j, std::shared_ptr<Client> c)
{
    j =
    {
        {"client_id", cid_to_hex(c->id)},
        {"drone_name", c->drone_name},
        {"serial", c->serial}
    };
}

void WebsocketClient::on_drone_list()
{
    json j =
    {
        {"type", "drone_list_resp"}
    };
    j["data"] = json(client_pool::get().drones());

    std::dynamic_pointer_cast<WebsocketServer>(server)->send(shared_from_this(), j.dump());
}