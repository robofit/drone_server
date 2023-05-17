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
        qs += "pitch DOUBLE,";
        qs += "roll DOUBLE,";
        qs += "yaw DOUBLE,";
        qs += "compass DOUBLE,";
        qs += "time TIMESTAMP DEFAULT CURRENT_TIMESTAMP";
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
            ins += j["data"]["Altitude"].dump();
            ins += ',';
            ins += j["data"]["Latitude"].dump();
            ins += ',';
            ins += j["data"]["Longitude"].dump();
            ins += ',';
            ins += j["data"]["Pitch"].dump();
            ins += ',';
            ins += j["data"]["Roll"].dump();
            ins += ',';
            ins += j["data"]["Yaw"].dump();
            ins += ',';
            ins += j["data"]["Compass"].dump();
            ins += ',';
            ins += "CURRENT_TIMESTAMP";
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