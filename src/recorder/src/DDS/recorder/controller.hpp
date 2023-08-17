#ifndef RECORDER_CONTROLLER_HPP
#define RECORDER_CONTROLLER_HPP

#include <DDS/core/flight_data/string_handler.hpp>
#include <DDS/recorder/recorder.hpp>
#include <map>

class RecorderController : public StringHandler
{
public:
    bool handle_string(const ClientID_t, std::string const&);
private:
    std::map<ClientID_t, std::shared_ptr<media_recorder>> recorders;
};

#endif