#ifndef FLIGHT_DATABASE_HPP
#define FLIGHT_DATABASE_HPP

#include <DDS/core/flight_data/string_handler.hpp>

namespace mariadb {
    class connection;
}
class FlightDatabase : public StringHandler
{
public:
    FlightDatabase();
    bool handle_string(const ClientID_t, std::string const&);
private:
    std::shared_ptr<mariadb::connection> con;
};

#endif