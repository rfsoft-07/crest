#include <boost/asio.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <iostream>
#include <string>

namespace beast = boost::beast;
namespace http = beast::http;
namespace asio = boost::asio;
using tcp = asio::ip::tcp;
using asio::awaitable;

// Handle a single client connection
awaitable<void> handle_client(tcp::socket socket)
{
    try
    {
        beast::flat_buffer buffer;
        http::request<http::string_body> req;

        // Read HTTP request asynchronously
        co_await http::async_read(socket, buffer, req, asio::use_awaitable);

        // Check for POST method
        if (req.method() == http::verb::post)
        {
            // Log request body (e.g., JSON sale data)
            std::cout << "Received POST: " << req.body() << std::endl;

            // Prepare 200 OK response
            http::response<http::string_body> res{http::status::ok, req.version()};
            res.set(http::field::server, "POS-Server");
            res.set(http::field::content_type, "application/json");
            res.body() = R"({"status": "success"})";
            res.prepare_payload();

            // Send response asynchronously
            co_await http::async_write(socket, res, asio::use_awaitable);
        }
        else
        {
            // Handle non-POST requests (optional)
            http::response<http::string_body> res{http::status::method_not_allowed, req.version()};
            res.set(http::field::server, "POS-Server");
            res.body() = R"({"error": "Only POST allowed"})";
            res.prepare_payload();
            co_await http::async_write(socket, res, asio::use_awaitable);
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "Client error: " << e.what() << std::endl;
    }
}

// Accept incoming connections
awaitable<void> start_server(tcp::acceptor &acceptor)
{
    while (true)
    {
        tcp::socket socket(acceptor.get_executor());
        co_await acceptor.async_accept(socket, asio::use_awaitable);
        co_spawn(acceptor.get_executor(), handle_client(std::move(socket)), asio::detached);
    }
}

int main()
{
    try
    {
        asio::io_context io_context;
        tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(), 8080));
        std::cout << "POS Server running on localhost:8080" << std::endl;

        // Start accepting connections
        co_spawn(io_context, start_server(acceptor), asio::detached);
        io_context.run();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Server error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}