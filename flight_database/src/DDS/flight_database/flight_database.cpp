#include <DDS/flight_database/flight_database.hpp>
#include <DDS/websocket/json.hpp>
#include <DDS/core/settings.hpp>

#include <mariadb++/connection.hpp>

using json = nlohmann::json;

FlightDatabase::FlightDatabase()
{
    settings& sett = settings::get();

    // set up the account
    mariadb::account_ref acc = mariadb::account::create(sett.dstring["mariadb_hostname"], sett.dstring["mariadb_username"], sett.dstring["mariadb_password"], sett.dstring["mariadb_database"]);

    // create connection
    con = mariadb::connection::create(acc);

    std::string qs = "SELECT count(*) FROM information_schema.tables WHERE table_schema = ";
    qs += '\'' + sett.dstring["mariadb_database"] + '\'';
    qs += " AND table_name = ";
    qs += "\'flight_data\';";

    auto res = con->query(qs);
    if(res->next() && res->get_signed64(0) == 0)
    {
        qs  = "CREATE TABLE flight_data (";
        qs += "client_id char(8),";
        qs += "altitude DOUBLE,";
        qs += "latitude DOUBLE,";
        qs += "longitude DOUBLE,";
        qs += "aircraft_pitch DOUBLE,";
        qs += "aircraft_roll DOUBLE,";
        qs += "aircraft_yaw DOUBLE,";
        qs += "aircraft_compass DOUBLE,";
        qs += "aircraft_velocity_x DOUBLE,";
        qs += "aircraft_velocity_y DOUBLE,";
        qs += "aircraft_velocity_z DOUBLE,";
        qs += "gimbal_pitch DOUBLE,";
        qs += "gimbal_roll DOUBLE,";
        qs += "gimbal_yaw DOUBLE,";
        qs += "gimbal_yaw_relative DOUBLE,";
        qs += "timestamp TIMESTAMP";
        qs += ");";

        res = con->query(qs);
    }
}

bool FlightDatabase::handle_string(const ClientID_t, std::string const& msg)
{
    try
    {
        json j = json::parse(msg);

        if(j["type"] == "data_broadcast")
        {
            std::string ins = "INSERT INTO flight_data VALUES (\'";
            ins += j["data"]["client_id"];
            ins += "\',";
            ins += j["data"]["altitude"].dump();
            ins += ',';
            ins += j["data"]["gps"]["latitude"].dump();
            ins += ',';
            ins += j["data"]["gps"]["longitude"].dump();
            ins += ',';
            ins += j["data"]["aircraft_orientation"]["pitch"].dump();
            ins += ',';
            ins += j["data"]["aircraft_orientation"]["roll"].dump();
            ins += ',';
            ins += j["data"]["aircraft_orientation"]["yaw"].dump();
            ins += ',';
            ins += j["data"]["aircraft_orientation"]["compass"].dump();
            ins += ',';
            ins += j["data"]["aircraft_velocity"]["velocity_x"].dump();
            ins += ',';
            ins += j["data"]["aircraft_velocity"]["velocity_y"].dump();
            ins += ',';
            ins += j["data"]["aircraft_velocity"]["velocity_z"].dump();
            ins += ',';
            ins += j["data"]["gimbal_orientation"]["pitch"].dump();
            ins += ',';
            ins += j["data"]["gimbal_orientation"]["roll"].dump();
            ins += ',';
            ins += j["data"]["gimbal_orientation"]["yaw"].dump();
            ins += ',';
            ins += j["data"]["gimbal_orientation"]["yaw_relative"].dump();
            ins += ",\'";
            ins += j["data"]["timestamp"];
            ins += '\'';
            ins += ");";

            con->insert(ins);
            return true;
        }
    }
    catch (json::exception const& ex)
    {
        //no need to handle - just return false;
    }
    return false;
}