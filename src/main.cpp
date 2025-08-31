#include <boost/asio.hpp>
#include <iostream>

void handle_receive(const boost::system::error_code &ec, std::size_t byte_transferred)
{
    if (!ec)
    {
        std::cout << "received" << byte_transferred << "bytes\n";
    }
}

int main()
{
    boost::asio::io_context io_context;
}