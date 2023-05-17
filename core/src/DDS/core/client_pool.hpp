#ifndef CLIENT_POOL_HPP
#define CLIENT_POOL_HPP

#include <map>
#include <vector>
#include <algorithm>
#include <DDS/core/client.hpp>

class client_pool
{
public:
    client_pool(client_pool const&) = delete;
    void operator=(client_pool const&) = delete;
    static client_pool& get()
    {
        static client_pool instance;
        return instance;
    }

    void add(std::shared_ptr<Client> cp)
    {
        clients_.push_back(cp);
        if(cp->type == Client::Type::DRONE)
            drones_.push_back(cp);
        client_ids_[cp->id] = cp;
    }
    void del(std::shared_ptr<Client> cp)
    {
        clients_.erase(std::remove(clients_.begin(), clients_.end(), cp), clients_.end());
        if(cp->type == Client::Type::DRONE)
            drones_.erase(std::remove(drones_.begin(), drones_.end(), cp), drones_.end());
        if(client_ids_[cp->id])
            client_ids_.erase(cp->id);
    }

    auto clients() const { return clients_; }
    auto drones() const { return drones_; }
    auto count(ClientID_t id) const { return client_ids_.count(id); }
    std::shared_ptr<Client> client(ClientID_t id) const { return client_ids_.at(id); }
private:
    client_pool() {}
    std::vector<std::shared_ptr<Client>> clients_;
    std::vector<std::shared_ptr<Client>> drones_;
    std::map<ClientID_t, std::shared_ptr<Client>> client_ids_;
};

#endif