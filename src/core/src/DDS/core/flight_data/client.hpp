#ifndef FLIGHT_DATA_CLIENT_HPP
#define FLIGHT_DATA_CLIENT_HPP

#include <string>
#include <DDS/core/client.hpp>
#include <DDS/core/flight_data/server.hpp>

class FlightDataClient : public Client
{
public:
	virtual ~FlightDataClient() {}
	virtual void recv(std::string) {}
	virtual void send(std::string const&) {}
protected:
	std::shared_ptr<FlightDataServer> server;
	FlightDataClient(std::shared_ptr<FlightDataServer> s)
	: server(s) {};	
};

#endif // !FLIGHT_DATA_CLIENT_HPP
