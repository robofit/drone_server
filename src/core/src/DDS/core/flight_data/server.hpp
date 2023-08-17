#ifndef FLIGHT_DATA_SERVER_HPP
#define FLIGHT_DATA_SERVER_HPP

#include <cstdint>
#include <memory>

class FlightDataServer : public std::enable_shared_from_this<FlightDataServer>
{
public:
	virtual ~FlightDataServer() {}

	virtual void run(uint16_t port) {}
protected:
	FlightDataServer() {}
};

#endif // !FLIGHT_DATA_SERVER_HPP
