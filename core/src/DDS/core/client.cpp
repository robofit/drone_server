#include <DDS/core/client.hpp>
#include <cstdlib>

class random_unsigned
{
public:
    random_unsigned(random_unsigned const&) = delete;
	void operator=(random_unsigned const&) = delete;
    static unsigned gen()
    {
        static random_unsigned instance;
        return static_cast<unsigned>(std::rand());
    }
private:
    random_unsigned()
    {
        std::srand(static_cast<unsigned>(std::time(0)));
    }
};

Client::Client()
: id(random_unsigned::gen())
{

}