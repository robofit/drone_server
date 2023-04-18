#include <DDS/core/flight_data/client.hpp>
#include <DDS/core/flight_data/server.hpp>
#include <cstdlib>

std::vector<Client*> drones_;

class random_unsigned
{
public:
    random_unsigned(random_unsigned const&) = delete;
	void operator=(random_unsigned const&) = delete;
    static unsigned gen()
    {
        static random_unsigned instance;
        return static_cast<unsigned>(std::rand());
    }
private:
    random_unsigned()
    {
        std::srand(static_cast<unsigned>(std::time(0)));
    }
};

FlightDataClient::FlightDataClient(std::shared_ptr<FlightDataServer> s)
: server(s)
{
    
}

void FlightDataClient::hello(unsigned type, std::string drone, std::string serial)
{
    if (handshake_done)
        return;

    this->type = static_cast<Client::Type>(type);
    this->id = random_unsigned::gen();
    this->drone_name = drone;
    this->serial = serial;

    if (this->type == Client::Type::DRONE)
        drones_.push_back(this);

    handshake_done = true;
    on_hello(id);
}

void FlightDataClient::data(std::string msg)
{
    if (!handshake_done)
        return;

    server->broadcast(this, msg);
    on_data();
}

void FlightDataClient::drone_list()
{
    if (!handshake_done)
        return;

    on_drone_list(drones_);
}
