#ifndef WEBSOCKET_CLIENT_HPP
#define WEBSOCKET_CLIENT_HPP

#include <string>
#include <DDS/core/async_handler.hpp>
#include <DDS/core/flight_data/client.hpp>
#include <DDS/core/flight_data/server.hpp>

class WebsocketClient : public FlightDataClient, async_handler<std::string>
{
public:
    WebsocketClient(std::shared_ptr<FlightDataServer>);
    ~WebsocketClient();
    void recv(std::string);
    void send(std::string const&);
private:
    void handle(std::string);

	void on_hello();
	void on_data(std::string const&);
	void on_drone_list();

    bool handshake_done = false;
};

#endif