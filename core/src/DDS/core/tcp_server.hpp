#ifndef TCP_SERVER_HPP
#define TCP_SERVER_HPP

#include <boost/asio.hpp>
#include <boost/asio/strand.hpp>
#include <boost/static_assert.hpp>
#include <boost/type_traits/is_base_of.hpp>

#include <DDS/core/logger.hpp>
#include <DDS/core/tcp_session.hpp>

template<class S>
class tcp_server : public std::enable_shared_from_this<tcp_server<S>>
{
    using tcp = boost::asio::ip::tcp;
public:
    tcp_server(boost::asio::io_context& io_context)
    : ioc(io_context), acceptor(io_context)
    {
        BOOST_STATIC_ASSERT(boost::is_base_of<tcp_session, S>::value);
    }
    void run(uint16_t port, tcp prot = tcp::v4())
    {
        boost::system::error_code ec;
        tcp::endpoint endpoint(prot, port);

        acceptor.open(endpoint.protocol(), ec);
        acceptor.set_option(tcp::acceptor::reuse_address(true));
        acceptor.bind(endpoint, ec);
        acceptor.listen(boost::asio::socket_base::max_listen_connections, ec);

        accept();
    }
private:
    boost::asio::io_context& ioc;
    tcp::acceptor acceptor;

    void accept()
    {
        auto self(this->shared_from_this());
        acceptor.async_accept(boost::asio::make_strand(ioc), [self](boost::system::error_code ec, tcp::socket socket)
        {
            if (!ec)
                std::make_shared<S>(std::move(socket))->run();
            else
            {
                LOG(ERROR) << "<core> " << "tcp_server accept error: " << ec.message();
                return;
            }
            self->accept();
        });
    }
};

#endif
