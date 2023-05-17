#ifndef RECORDER_CONTROLLER_HPP
#define RECORDER_CONTROLLER_HPP

#include <DDS/core/flight_data/string_handler.hpp>

class RecorderController : public StringHandler
{
public:
    bool handle_string(const ClientID_t, std::string const&);
};

#endif