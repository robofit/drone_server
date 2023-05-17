#ifndef VEHICLE_DETECTION_CONTROLLER_HPP
#define VEHICLE_DETECTION_CONTROLLER_HPP

#include <map>
#include <memory>
#include <DDS/core/flight_data/string_handler.hpp>
#include <DDS/vehicle_detection/vehicle_detection.hpp>

class VehicleDetectionController : public StringHandler
{
public:
    bool handle_string(const ClientID_t, std::string const&);
    std::map<ClientID_t, std::shared_ptr<VehicleDetector>> detectors;
};

#endif