#ifndef FLIGHT_DATA_CLIENT_HPP
#define FLIGHT_DATA_CLIENT_HPP

#include <string>
#include <vector>
#include <memory>
#include <DDS/core/client.hpp>
#include <DDS/core/flight_data/server.hpp>

class FlightDataClient : public Client
{
public:
	virtual ~FlightDataClient() {}
	virtual void recv(std::string) {}
protected:
	FlightDataClient(std::shared_ptr<FlightDataServer> s);

	void hello(unsigned type, std::string drone = "", std::string serial = "");
	void data(std::string msg);
	void drone_list();

	virtual void on_hello(ClientID_t) {}
	virtual void on_data() {}
	virtual void on_drone_list(std::vector<Client*>&) {}

	std::shared_ptr<FlightDataServer> server;

	bool handshake_done = false;
};

#endif // !FLIGHT_DATA_CLIENT_HPP
