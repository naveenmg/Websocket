
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <string>
#include <thread>
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

//------------------------------------------------------------------------------

// Echoes back all received WebSocket messages
void
do_session(tcp::socket& socket)
{
	try
	{
		// Construct the stream by moving in the socket
		websocket::stream<tcp::socket> ws{ std::move(socket) };

		// Set a decorator to change the Server of the handshake
		ws.set_option(websocket::stream_base::decorator(
			[](websocket::response_type& res)
		{
			res.set(http::field::server,
				std::string(BOOST_BEAST_VERSION_STRING) +
				" websocket-server-sync");
		}));

		// Accept the websocket handshake
		ws.accept();

		for (;;)
		{
			// This buffer will hold the incoming message
			beast::flat_buffer buffer;

			// Read a message
			ws.read(buffer);
			//			ws.text(ws.got_text());
			//			ws.write(buffer.data());
			std::string trail(beast::buffers_to_string(buffer.data()));
			if ( trail == "New Temperature Data Set") 
			{
				buffer.consume(buffer.size());
				ws.read(buffer);
				std::string data_chg(beast::buffers_to_string(buffer.data()));
				
				std::ifstream ifs{ R"(D:\OneDrive - Hochschule Wismar\MASTER\Sem3\WatttronGmbh\Projekt\Server\test.json)" };

				IStreamWrapper isw{ ifs };

				Document doc{};
				doc.ParseStream(isw);

				// 3. Stringify the DOM
				StringBuffer buffer1{};
				Writer<StringBuffer> writer{ buffer1 };
				doc.Accept(writer);

				const std::string jsonStr{ buffer1.GetString() };
				std::cout << jsonStr << '\n';
				doc["num"] = std::stoi(data_chg);
				std::ofstream ofs{ R"(D:\OneDrive - Hochschule Wismar\MASTER\Sem3\WatttronGmbh\Projekt\Server\test.json)" };

				if (!ofs.is_open())
				{
					std::cerr << "Could not open file for writing!\n";
				}

				OStreamWrapper osw{ ofs };
				Writer<OStreamWrapper> writer2{ osw };
				doc.Accept(writer2);
				boost::system::error_code error;
		//		buffer.consume(buffer.size());
				net::const_buffer b("value changed", 13);
				ws.text(true);
				// Write the buffer as text
				ws.write(b);
				std::cout << "Server sent reply" << std::endl;
	//			const std::string jsonStr{ buffer1.GetString() };
	//			boost::system::error_code error;
	//			ws.write(net::buffer(std::string(jsonStr.c_str())), error);
	//			if (!error) {
	//				std::cout << "Server sent new data" << std::endl;
	//				std::cout << beast::make_printable(buffer.data()) << std::endl;
	//				ws.write(buffer.data());
	//			}
	//			else
	//		{
	//				std::cout << "reply failed:" << error.message() << std::endl;
	//			}
				
				}
			else
			{
				std::ifstream ifs{ R"(D:\OneDrive - Hochschule Wismar\MASTER\Sem3\WatttronGmbh\Projekt\Server\test.json)" };

				IStreamWrapper isw{ ifs };


				Document doc{};
				doc.ParseStream(isw);

				// 2. Modify it by DOM.
		//		Value& s = d["stars"];
		//		s.SetInt(s.GetInt() + 1);

				// 3. Stringify the DOM
				StringBuffer buffer1{};
				Writer<StringBuffer> writer{ buffer1 };
				doc.Accept(writer);



				const std::string jsonStr{ buffer1.GetString() };

				//	auto const text = jsonStr;
				boost::system::error_code error;
				ws.write(net::buffer(std::string(jsonStr.c_str())), error);
				if (!error) {
					std::cout << "Server sent reply" << std::endl;
					std::cout << beast::make_printable(buffer.data()) << std::endl;
					ws.write(buffer.data());
				}
				else
				{
					std::cout << "reply failed:" << error.message() << std::endl;
				}
			}
		}
	}
	catch (beast::system_error const& se)
	{
		// This indicates that the session was closed
		if (se.code() != websocket::error::closed)
			std::cerr << "Error: " << se.code().message() << std::endl;
	}
	catch (std::exception const& e)
	{
		std::cerr << "Error: " << e.what() << std::endl;
	}
}

//------------------------------------------------------------------------------

int main(int argc, char* argv[])
{
	try
	{
		
		// Check command line arguments.
		if (argc != 3)
		{
			std::cerr <<
				"wrong arguments";
			return EXIT_FAILURE;
		}
		auto const address = net::ip::make_address(argv[1]);
		auto const port = static_cast<unsigned short>(std::atoi(argv[2]));
		
		// The io_context is required for all I/O
		net::io_context ioc{ 1 };

		// The acceptor receives incoming connections
		tcp::acceptor acceptor{ ioc, {address, port} };
		for (;;)
		{
			// This will receive the new connection
			tcp::socket socket{ ioc };

			// Block until we get a connection
			acceptor.accept(socket);

			// Launch the session, transferring ownership of the socket
			std::thread{ std::bind(
				&do_session,
				std::move(socket)) }.detach();
		}
	}
	catch (const std::exception & e)
	{
		std::cerr << "Error: " << e.what() << std::endl;
		return EXIT_FAILURE;
	}
}