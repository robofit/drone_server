#ifndef FLIGHT_DATA_MANAGER_HPP
#define FLIGHT_DATA_MANAGER_HPP

#include <map>
#include <DDS/core/flight_data/string_handler.hpp>

class flight_data_manager
{
public:
    flight_data_manager(flight_data_manager const&) = delete;
    void operator=(flight_data_manager const&) = delete;
    static flight_data_manager& get()
    {
        static flight_data_manager instance;
        return instance;
    }

    bool handle(std::string const& msg) const
    {
        bool ret = false;
        for(auto& pair : recs)
        {
            ret |= pair.second->handle_string(pair.first, msg);
        }
        return ret;
    }

    void add(const ClientID_t cid, std::shared_ptr<StringHandler> sh_ptr)
    {
        recs[cid] = sh_ptr;
    }
    void del(const ClientID_t cid)
    {
        recs.erase(cid);
    }
private:
    std::map<ClientID_t, std::shared_ptr<StringHandler>> recs;
    flight_data_manager() {}
};


#endif // !FLIGHT_DATA_MANAGER_HPP
