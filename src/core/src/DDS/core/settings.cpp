#include <DDS/core/settings.hpp>
#include <DDS/core/logger.hpp>

structlog LOGCFG{};

settings::settings()
{

}

void settings::set_loglevel(int l)
{
    LOGCFG.level = static_cast<typelog>(l);
}
