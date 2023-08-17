#ifndef MEDIA_MANAGER_HPP
#define MEDIA_MANAGER_HPP

#include <map>
#include <DDS/core/media/copypipe.hpp>

class media_manager
{
public:
    media_manager(media_manager const&) = delete;
    void operator=(media_manager const&) = delete;
    static media_manager& get()
    {
        static media_manager instance;
        return instance;
    }

    std::shared_ptr<media_copypipe> pipe(ClientID_t cid)
    {
        if(recs.count(cid) == 0)
            recs[cid] = std::make_shared<media_copypipe>(cid);

        return recs[cid];
    }
    void erase(ClientID_t cid)
    {
        if(recs.count(cid) > 0)
            recs.erase(cid);
    }
    void clear()
    {
        recs.clear();
    }
private:
    std::map<ClientID_t, std::shared_ptr<media_copypipe>> recs;
    media_manager() {}
};


#endif // !MEDIA_MANAGER_HPP
