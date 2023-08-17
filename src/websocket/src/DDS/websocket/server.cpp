#include <DDS/websocket/server.hpp>
#include <DDS/websocket/client.hpp>
#include <DDS/core/client_pool.hpp>
#include <DDS/core/flight_data/client.hpp>
#include <DDS/core/logger.hpp>

#include <functional>

WebsocketServer::WebsocketServer(boost::asio::io_context* io)
{
    server.clear_access_channels(websocketpp::log::alevel::all);
    server.init_asio(io);
    server.set_reuse_addr(true);
}

WebsocketServer::~WebsocketServer()
{
    server.stop_listening();
    for (auto& c : clients)
    {
        //server.close(c.first, websocketpp::close::status::normal, "finished");//could be dangerous on SIGINT - dont send close
    }
}

void WebsocketServer::run(uint16_t port)
{
    auto self(std::dynamic_pointer_cast<WebsocketServer>(shared_from_this()));

    server.set_open_handler(std::bind(&WebsocketServer::on_open, self, std::placeholders::_1));
    server.set_close_handler(std::bind(&WebsocketServer::on_close, self, std::placeholders::_1));
    server.set_message_handler(std::bind(&WebsocketServer::on_message, self, std::placeholders::_1, std::placeholders::_2));
    server.set_fail_handler(std::bind(&WebsocketServer::on_fail, self, std::placeholders::_1));

    server.listen(port);
    server.start_accept();

    LOG(INFO) << "<websocket> " << "starting server on port: " << port;
}

void WebsocketServer::send(std::shared_ptr<Client> dst, const std::string& msg)
{
    for (auto& c : clients)
    {
        if (c.second == dst)
        {
            try
            {
                server.send(c.first, msg, websocketpp::frame::opcode::TEXT);
            }
            catch (websocketpp::exception const& e)
            {
                LOG(ERROR) << "<websocket> " << e.what();
                kick(dst, websocketpp::close::status::bad_gateway, e.what());
            }
        }
    }
}


void WebsocketServer::kick(std::shared_ptr<Client> dst, websocketpp::close::status::value status, std::string reason)
{
    for (auto& c : clients)
    {
        if (c.second == dst)
        {
            server.close(c.first, status, reason);
        }
    }
}

void WebsocketServer::on_open(websocketpp::connection_hdl conn)
{
    if (clients[conn])
        clients.erase(conn);

    LOG(INFO) << "<websocket> " << "new client connected";
    clients[conn] = std::make_shared<WebsocketClient>(shared_from_this());
}

void WebsocketServer::on_close(websocketpp::connection_hdl conn)
{
    if (clients[conn])
    {
        client_pool::get().del(clients[conn]);
        clients.erase(conn);
    }

    LOG(INFO) << "<websocket> " << "client disconnected";
}

void WebsocketServer::on_message(websocketpp::connection_hdl conn, server_t::message_ptr msg)
{
    if (clients[conn])
    {
        LOG(DEBUG) << "<websocket> " << "data income: " << msg->get_payload();
        std::dynamic_pointer_cast<FlightDataClient>(clients[conn])->recv(msg->get_payload());
    }
}

void WebsocketServer::on_fail(websocketpp::connection_hdl conn)
{
    LOG(ERROR) << "<websocket> " << "connection failed";
    if (clients[conn])
    {
        client_pool::get().del(clients[conn]);
        clients.erase(conn);
    }
}
