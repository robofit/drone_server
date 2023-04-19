#ifndef TCP_SESSION_HPP
#define TCP_SESSION_HPP

#include <boost/asio.hpp>
#include <boost/asio/dispatch.hpp>
#include <queue>
#include <memory>
#include <functional>

#include <DDS/core/logger.hpp>

class tcp_session : public std::enable_shared_from_this<tcp_session>
{
protected:
    using tcp = boost::asio::ip::tcp;

    virtual void on_connect() {}
    virtual void on_write(size_t) {}
    virtual void on_read(char*, size_t) {}
    virtual void on_close() {}
    virtual void on_error(boost::system::error_code ec)
    {
        LOG(ERROR) << "<core> tcp_session " << ec.message();
        close();
    }
public:
    tcp_session(tcp::socket socket)
    : socket_(std::move(socket))
    {
        read_buffer_ = new char[read_buffer_size];
    }
    ~tcp_session()
    {
        close();
        delete[] read_buffer_;
    }
    
    void run()
    {
        boost::asio::dispatch(socket_.get_executor(), std::bind(&tcp_session::on_connect, shared_from_this()));
    }
    void close()
    {
        if (socket_.is_open())
        {
            socket_.close();
            on_close();
        }
    }

    void async_write(char* data, size_t size, bool flush = true)
    {
        send_queue.push(std::make_shared<std::vector<uint8_t>>(data, data + size));
        if(flush)
            do_write();
    }
    void async_write(std::shared_ptr<std::vector<uint8_t>> data, bool flush = true)
    {
        send_queue.push(std::make_shared<std::vector<uint8_t>>(*data));
        if (flush)
            do_write();
    }
    void async_read(size_t read_buf_size = read_buffer_size)
    {
        if(read_buf_size > read_buffer_size)
            return;
        auto self(shared_from_this());
        socket_.async_read_some(boost::asio::buffer(read_buffer_, read_buf_size), [self](boost::system::error_code ec, size_t read_size)
        {
            if(!ec)
            {
                self->on_read(self->read_buffer_, read_size);
            }
            else
            {
                self->on_error(ec);
            }
        });
    }
    void read(void* data, size_t& size)
    {
        size = socket_.read_some(boost::asio::buffer(data, size));
    }
    void write(const void* data, size_t& size)
    {
        size = socket_.write_some(boost::asio::buffer(data, size));
    }
    bool alive() const { return socket_.is_open(); }
private:
    tcp::socket socket_;
    std::queue<std::shared_ptr<std::vector<uint8_t>>> send_queue;
    constexpr static size_t read_buffer_size = 4096 * 2;
    char* read_buffer_;
    bool sending_ = false;

    void do_write()
    {
        if(!socket_.is_open())
        {
            close();
            return;
        }

        if (send_queue.empty() || sending_)
            return;

        auto msg_p = send_queue.front();
        sending_ = true;

        auto self(shared_from_this());
        boost::asio::async_write(socket_, boost::asio::buffer(msg_p->data(), msg_p->size()), boost::asio::bind_executor(socket_.get_executor(),
        [self](boost::system::error_code ec, size_t write_size)
        {
            if(!ec)
            {
                self->send_queue.pop();
                self->sending_ = false;
                self->on_write(write_size);
                self->do_write();
            }
            else
            {
                self->on_error(ec);
            }
        }));
    }
};

#endif