#ifndef STRING_HANDLER_HPP
#define STRING_HANDLER_HPP

#include <DDS/core/client.hpp>

class StringHandler
{
public:
    virtual bool handle_string(const ClientID_t, std::string const&) { return false; }
};

#endif