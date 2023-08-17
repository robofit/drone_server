#ifndef WEBSOCKET_SERVER_HPP
#define WEBSOCKET_SERVER_HPP

#include <map>
#include <boost/asio.hpp>
#include <websocketpp/server.hpp>
#include <websocketpp/config/asio_no_tls.hpp>
#include <DDS/core/flight_data/server.hpp>
#include <DDS/core/flight_data/client.hpp>

class WebsocketServer : public FlightDataServer
{
	using server_t = websocketpp::server<websocketpp::config::asio>;
public:
	WebsocketServer(boost::asio::io_context*);
	~WebsocketServer();

	void run(uint16_t);
	void send(std::shared_ptr<Client>, const std::string& msg);
	void kick(std::shared_ptr<Client>, websocketpp::close::status::value, std::string reason);
private:
	server_t server;

	std::map<websocketpp::connection_hdl, std::shared_ptr<FlightDataClient>, std::owner_less<websocketpp::connection_hdl>> clients;

	void on_open(websocketpp::connection_hdl);
	void on_close(websocketpp::connection_hdl);
	void on_message(websocketpp::connection_hdl, server_t::message_ptr);
	void on_fail(websocketpp::connection_hdl);
};

#endif // !SERVER_HPP
