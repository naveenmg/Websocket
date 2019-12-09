#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <cstdlib>
#include <iostream>
#include <string>
#include<rapidjson/document.h>
#include<rapidjson/writer.h>
#include<rapidjson/stringbuffer.h>
#include <fstream>
#include <rapidjson/istreamwrapper.h>
#include<rapidjson/ostreamwrapper.h>



namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace websocket = beast::websocket; // from <boost/beast/websocket.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>
using namespace rapidjson;


// Sends a WebSocket message and prints the response
int main()
{
	try
	{
		
		auto const host = "127.0.0.1";
	//	std::string const host = "echo.websocket.org";
		auto const port = "1234";
		std::string const text = "Temperature Data Request";
		std::string const change = "New Temperature Data Set";
		int i;
		int value;
		// The io_context is required for all I/O
		net::io_context ioc;

		// These objects perform our I/O
		tcp::resolver resolver{ ioc };
		websocket::stream<tcp::socket> ws{ ioc };

		// Look up the domain name
		auto const results = resolver.resolve(host,port);

		// Make the connection on the IP address we get from a lookup
		net::connect(ws.next_layer(), results.begin(), results.end());

		// Set a decorator to change the User-Agent of the handshake
		ws.set_option(websocket::stream_base::decorator(
			[](websocket::request_type& req)
		{
			req.set(http::field::user_agent,
				std::string(BOOST_BEAST_VERSION_STRING) +
				" websocket-client-coro");
		}));

		// Perform the websocket handshake
		ws.handshake(host, "/");

		// Send the message
		boost::system::error_code error;
		std::cout << "Please enter 1 to request temperature data and 2 to set new data: ";
		std::cin >> i;
		if (i == 1) {
			ws.write(net::buffer(std::string(text)), error);
			if (!error) {
				std::cout << "Client sent message" << std::endl;
			}
			else
			{
				std::cout << "send failed:" << error.message() << std::endl;
			}
			beast::flat_buffer buffer;
			ws.read(buffer);	
			std::cout << beast::buffers_to_string(buffer.data()) << std::endl;
		}
		else
		{
			ws.write(net::buffer(std::string(change)), error);

			if (!error) {
				std::cout << "Client sent message" << std::endl;
			}
			else
			{
				std::cout << "send failed:" << error.message() << std::endl;
			}
			//beast::flat_buffer buffer;
			//ws.read(buffer);
			std::cout << "ENTER NEW AGENT NUMBER: ";
			std::cin >> value;
			ws.write(net::buffer(std::to_string(value)), error);
			if (!error) {
				std::cout << "Client send new data" << std::endl;
			}
			else
			{
				std::cout << "new data send failed:" << error.message() << std::endl;
			}
			
		
		}
		

		// Close the WebSocket connection
		ws.close(websocket::close_code::normal);

		
		//	}
	}
	catch (std::exception const& e)
	{
		std::cerr << "Error: " << e.what() << std::endl;
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}