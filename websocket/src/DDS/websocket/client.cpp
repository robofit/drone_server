#include <DDS/websocket/client.hpp>
#include <DDS/websocket/server.hpp>
#include <DDS/websocket/json.hpp>

#include <DDS/core/logger.hpp>
#include <DDS/core/settings.hpp>
#include <DDS/core/flight_data/server.hpp>

#include <websocketpp/close.hpp>

using json = nlohmann::json;

WebsocketClient::WebsocketClient(std::shared_ptr<FlightDataServer> s)
    : FlightDataClient(s)
{

}

void WebsocketClient::recv(std::string msg)
{
    add(msg);
}

void WebsocketClient::handle(std::string msg)
{
    try
    {
        json j = json::parse(msg);

        if(j["type"] == "hello" && !handshake_done)
        {
            int ctype = j["data"]["ctype"];
            if(ctype == 0)
            {
                hello(ctype, j["data"]["drone_name"], j["data"]["serial"]);
            }
            else if(ctype == 1)
            {
                hello(ctype);
            }
            
        }
        else if(j["type"] == "data_broadcast" && handshake_done)
        {
            data(j.dump());
        }
        else if(j["type"] == "drone_list" && handshake_done)
        {
            drone_list();
        }
        else
        {
            LOG(ERROR) << "<websocket> " << "unsupported request";
            std::dynamic_pointer_cast<WebsocketServer>(server)->kick(this, websocketpp::close::status::invalid_payload, "unsupported request");
        }
    }
    catch (json::exception const& ex)
    {
        LOG(ERROR) << "<websocket> " << "parsing error";
        std::dynamic_pointer_cast<WebsocketServer>(server)->kick(this, websocketpp::close::status::invalid_payload, "parsing error");
    }
}

void WebsocketClient::on_hello(ClientID_t cid)
{
    json j =
    {
        {"type", "hello_resp"},
        {"data",
            {
                {"client_id", cid_to_hex(cid)}
            }
        }
    };

    if(this->type == Client::Type::OPERATOR)
    {
        j["data"]["rtmp_port"] = settings::get().dint.count("rtmp_port") > 0 ? settings::get().dint["rtmp_port"] : 0;
    }

    server->send(this, j.dump());
}

void WebsocketClient::on_data()
{

}

void to_json(json& j, const Client* c)
{
    j =
    {
        {"client_id", cid_to_hex(c->id)},
        {"drone_name", c->drone_name},
        {"serial", c->serial}
    };
}

void WebsocketClient::on_drone_list(std::vector<Client*>& drones)
{
    json j =
    {
        {"type", "drone_list_resp"}
    };
    j["data"] = json(drones);

    server->send(this, j.dump());
}