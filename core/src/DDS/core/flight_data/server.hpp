#ifndef FLIGHT_DATA_SERVER_HPP
#define FLIGHT_DATA_SERVER_HPP

#include <string>

class FlightDataClient;
class FlightDataServer : public std::enable_shared_from_this<FlightDataServer>
{
public:
	FlightDataServer() {}
	virtual ~FlightDataServer() {}

	virtual void run(uint16_t port) {}
	virtual void send(FlightDataClient* dst, const std::string& msg) {}
	virtual void broadcast(FlightDataClient* src, const std::string& msg) {}
};

#endif // !FLIGHT_DATA_SERVER_HPP
