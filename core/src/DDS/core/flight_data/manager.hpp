#ifndef FLIGHT_DATA_MANAGER_HPP
#define FLIGHT_DATA_MANAGER_HPP

#include <set>
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

    bool handle(const ClientID_t cid, std::string const& msg) const
    {
        bool ret = false;
        for(auto& sh_ptr : recs)
        {
            ret |= sh_ptr->handle_string(cid, msg);
        }
        return ret;
    }

    void add(std::shared_ptr<StringHandler> sh_ptr)
    {
        recs.insert(sh_ptr);
    }
    void del(std::shared_ptr<StringHandler> sh_ptr)
    {
        if(recs.count(sh_ptr) > 0)
            recs.erase(sh_ptr);
    }
    void clear()
    {
        recs.clear();
    }
private:
    std::set<std::shared_ptr<StringHandler>> recs;
    flight_data_manager() {}
};


#endif // !FLIGHT_DATA_MANAGER_HPP
