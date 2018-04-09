// STD
#include <atomic>
#include <cstdlib>
#include <iostream>
#include <string>

// MIF
#include <mif/common/unused.h>
#include <mif/net/http/server.h>

int main(int argc, char const **argv)
{
    try
    {
        if (argc != 4)
        {
            std::cout << "Usage: ./http_echo_server <host> <port> <workers>" << std::endl;
            std::cout << "For example: ./http_echo_server 0.0.0.0 55555 8" << std::endl;
            return EXIT_FAILURE;
        }

        std::atomic_size_t counter{0};

        Mif::Net::Http::Server server{argv[1], argv[2], static_cast<std::uint16_t>(std::stoi(argv[3])),
                {Mif::Net::Http::Method::Type::Post},
                {
                    {
                        "/echo", [&] (Mif::Net::Http::IInputPack const &req, Mif::Net::Http::IOutputPack &resp)
                        {
                            auto data = req.GetData();
                            resp.SetData(std::move(data));
                            ++counter;
                        }
                    },
                    {
                        "/stat", [&] (Mif::Net::Http::IInputPack const &req, Mif::Net::Http::IOutputPack &resp)
                        {
                            Mif::Common::Unused(req);
                            auto const value = "Counter: " + std::to_string(counter) + "\n";
                            resp.SetData({std::begin(value), std::end(value)});
                        }
                    }
                }
            };

        Mif::Common::Unused(server);

        std::cout << "Press any 'Enter' for quit." << std::endl;
        std::cin.get();
    }
    catch (std::exception const &e)
    {
        std::cerr << "Error: " << e.what();
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
