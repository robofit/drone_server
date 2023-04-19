#ifndef WEBSOCKET_CLIENT_HPP
#define WEBSOCKET_CLIENT_HPP

#include <string>
#include <DDS/core/async_handler.hpp>
#include <DDS/core/flight_data/client.hpp>

class FlightDataServer;
class WebsocketClient : public FlightDataClient, async_handler<std::string>
{
public:
    WebsocketClient(std::shared_ptr<FlightDataServer>);
    ~WebsocketClient() {}
    void recv(std::string);
private:
    void handle(std::string);

	void on_hello(ClientID_t);
	void on_data();
	void on_drone_list(std::vector<Client*>&);
};

#endif