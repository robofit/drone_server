#ifndef FLIGHT_DATABASE_CONTROLLER_HPP
#define FLIGHT_DATABASE_CONTROLLER_HPP

#include <DDS/flight_database/flight_database.hpp>

class FlightDatabaseController : public StringHandler
{
public:
    bool handle_string(const ClientID_t, std::string const&);
private:
    std::shared_ptr<FlightDatabase> fd_ptr;
};

#endif