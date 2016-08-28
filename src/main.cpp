#include <iostream>
#include <functional>
#include <memory>
#include <vector>

#include <thread>
#include <chrono>

#include <boost/asio.hpp>
#include <boost/shared_array.hpp>
#include <boost/thread.hpp>

/*
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>

#include "mif/remote/ps.h"
#include "mif/remote/proxy_stub.h"
#include "mif/remote/serialization/serialization.h"
#include "mif/remote/serialization/boost/serialization.h"

struct ITest
{
    virtual ~ITest() = default;
    virtual void Print() = 0;
    virtual void SetVersion(int major, int minor) = 0;
    virtual void SetName(std::string const &name) = 0;
    virtual std::string  GetName() = 0;
    virtual int GetMajor() const = 0;
    virtual int GetMinor() const = 0;
};

MIF_REMOTE_PS_BEGIN(ITest)
    MIF_REMOTE_METHOD(Print)
    MIF_REMOTE_METHOD(SetVersion)
    MIF_REMOTE_METHOD(SetName)
    MIF_REMOTE_METHOD(GetName)
    MIF_REMOTE_METHOD(GetMajor)
    MIF_REMOTE_METHOD(GetMinor)
MIF_REMOTE_PS_END()

class Test final
    : public ITest
{
private:
    std::string m_name{"TestDefault"};
    int m_major{0};
    int m_minor{0};

    virtual void Print() override
    {
        std::cout << "[Test::Print] Name\"" << m_name << "\" Major " << m_major << " Minor " << m_minor << std::endl;
    }
    virtual void SetVersion(int major, int minor) override
    {
        std::cout << "[Test::SetVersion] New major " << major << " New minor " << minor << std::endl;
        m_major = major;
        m_minor = minor;
    }
    virtual void SetName(std::string const &name) override
    {
        std::cout << "[Test::SetName] New name \"" << name << "\"" << std::endl;
        m_name = name;
    }
    virtual std::string  GetName() override
    {
        std::cout << "[Test::GetName] Name \"" << m_name << "\"" << std::endl;
        return m_name;
    }
    virtual int GetMajor() const override
    {
        std::cout << "[Test::GetMajor] Major " << m_major << std::endl;
        return m_major;
    }
    virtual int GetMinor() const override
    {
        std::cout << "[Test::GetMinor] Minor " << m_minor << std::endl;
        return m_minor;
    }
};

class TestTransport;
TestTransport *p_transport = nullptr;

class TestTransport
{
public:
    virtual ~TestTransport() = default;
    TestTransport()
    {
    }

    using Buffer = std::vector<char>;
    using DataHandler = std::function<Buffer (Buffer &&)>;

    DataHandler m_handler;

    void SetHandler(DataHandler && handler)
    {
        p_transport = this;
        m_handler = std::move(handler);
    }

    Buffer Send(Buffer && buffer);

    Buffer Call(Buffer && buffer)
    {
        return m_handler(std::move(buffer));
    }
};

TestTransport::Buffer TestTransport::Send(Buffer && buffer)
{
    return p_transport->Call(std::move(buffer));
}

int main()
{
	try
	{
        using BoostSerializer = Mif::Remote::Serialization::Boost::Serializer<boost::archive::xml_oarchive>;
        using BoostDeserializer = Mif::Remote::Serialization::Boost::Deserializer<boost::archive::xml_iarchive>;
        using SerializerTraits = Mif::Remote::Serialization::SerializerTraits<BoostSerializer, BoostDeserializer>;
        using ProxyStub = ITest_PS<Mif::Remote::Proxy<SerializerTraits, TestTransport>, Mif::Remote::Stub<SerializerTraits, TestTransport>>;

        TestTransport proxyTransport;
        TestTransport stubTransport;
        ProxyStub::Stub stub("100500", std::make_shared<Test>(), std::move(stubTransport));
        stub.Init();
        ProxyStub::Proxy proxy("100500", std::move(proxyTransport));

        ITest &rps = proxy;
        std::cout << "Old name: " << rps.GetName() << std::endl;
        std::cout << "Old major: " << rps.GetMajor() << std::endl;
        std::cout << "Old minor: " << rps.GetMinor() << std::endl;
        std::cout << "--------------------------- RPC ---------------------------" << std::endl;
        rps.SetName("New name");
        rps.SetVersion(100, 500);
        std::cout << "--------------------------- RPC ---------------------------" << std::endl;
        std::cout << "New name: " << rps.GetName() << std::endl;
        std::cout << "New major: " << rps.GetMajor() << std::endl;
        std::cout << "New minor: " << rps.GetMinor() << std::endl;
    }
	catch (std::exception const &e)
	{
		std::cerr << "Error: " << e.what() << std::endl;
	}
	return 0;
}
*/

namespace Mif
{
    namespace Remote
    {
    }   // namespace Remote
}   // namespace Mif

namespace Mif
{
    namespace Common
    {

        class ThreadPool final
        {
        public:
            using Task = std::function<void ()>;

            ThreadPool(std::uint16_t count)
                : m_work{new boost::asio::io_service::work{m_ioService}}
            {
                if (!count)
                    throw std::invalid_argument{"[Mif::Common::ThreadPool] Thread count must be more than 0."};

                std::exception_ptr exception{};
                for ( ; count ; --count)
                {
                    m_threads.create_thread([this, &exception] ()
                            {
                                try
                                {
                                    m_ioService.run();
                                }
                                catch (std::exception const &e)
                                {
                                    exception = std::current_exception();
                                    // TODO: to log
                                    std::cerr << "Failed to run thread pool. Error: " << e.what() << std::endl;
                                }
                            }
                        );

                    if (exception)
                        break;
                }

                if (exception)
                {
                    if (count)
                        Stop();

                    try
                    {
                        std::rethrow_exception(exception);
                    }
                    catch (std::exception const &e)
                    {
                        throw std::runtime_error{"[Mif::Common::ThreadPool] Failed to run thread pool. "
                            "Error: " + std::string{e.what()}};
                    }
                }
            }

            ~ThreadPool()
            {
                Stop();
            }

            void Post(Task task)
            {
                m_ioService.post(task);
            }

        private:
            boost::asio::io_service m_ioService;
            std::unique_ptr<boost::asio::io_service::work> m_work;
            boost::thread_group m_threads;

            void Stop()
            {
                try
                {
                    m_work.reset();
                    m_ioService.post([this] () { m_ioService.stop(); });
                    m_threads.join_all();
                }
                catch (std::exception const &e)
                {
                    // TODO: to log
                    std::cerr << "[Mif::Common::ThreadPool::Stop] Failed to stop thread pool. Error: " << e.what() << std::endl;
                }
            }
        };

    }   // namespace Common
}   // namespace Mif

namespace Mif
{
    namespace Net
    {

        using Buffer = std::pair<std::size_t, boost::shared_array<char>>;

        struct IControl
        {
            virtual ~IControl() = default;
            virtual void CloseMe() = 0;
        };

        struct IPublisher
        {
            virtual ~IPublisher() = default;
            virtual void Publish(Buffer buffer) = 0;
        };

        struct ISubscriber
        {
            virtual ~ISubscriber() = default;
            virtual void OnData(Buffer buffer) = 0;
        };

        struct ISubscriberFactory
        {
            virtual ~ISubscriberFactory() = default;
            virtual std::shared_ptr<ISubscriber> Create(std::weak_ptr<IControl> control,
                                                        std::weak_ptr<IPublisher> publisher) = 0;
        };

        class Client
            : public std::enable_shared_from_this<Client>
            , public ISubscriber
        {
        public:
            virtual ~Client() = default;

            Client(std::weak_ptr<IControl> control, std::weak_ptr<IPublisher> publisher)
                : m_control(control)
                , m_publisher(publisher)
            {
            }

        private:
            std::weak_ptr<IControl> m_control;
            std::weak_ptr<IPublisher> m_publisher;

            // ISubscriber
            virtual void OnData(Buffer buffer) override final
            {
                //std::cout << std::this_thread::get_id() << std::endl;
                //std::cout << "Readed " << buffer.first << " bytes." << std::endl;
                //throw std::runtime_error{"OnData not implemented."};
                char const resp[] = "HTTP/1.x 200 OK\r\nContent-Length: 0\r\nConnection: Closed\r\n\r\n";
                boost::shared_array<char> r(new char [sizeof(resp)]);
                char *p = r.get();
                char const *s = &resp[0];
                while(*p++ = *s++);
                Buffer b = std::make_pair(sizeof(resp) - 1, r);
                m_publisher.lock()->Publish(b);
                m_control.lock()->CloseMe();
            }
        };

        class ClientFactory final
            : public ISubscriberFactory
        {
        public:
        private:
            // ISubscriberFactory
            virtual std::shared_ptr<ISubscriber> Create(std::weak_ptr<IControl> control,
                std::weak_ptr<IPublisher> publisher) override
            {
                return std::make_shared<Client>(control, publisher);
            }
        };

    }   // namespace Net
}   // namespace Mif

namespace Mif
{
    namespace Net
    {
        namespace Detail
        {

            class TCPSession final
                : public std::enable_shared_from_this<TCPSession>
                , public IPublisher
                , public IControl
            {
            public:
                TCPSession(boost::asio::ip::tcp::socket socket,
                    Common::ThreadPool &workers, ISubscriberFactory &factory)
                    : m_socket(std::move(socket))
                    , m_workers(workers)
                    , m_factory(factory)
                {
                    //std::cout << "TCPSession" << std::endl;
                }
                ~TCPSession()
                {
                    //std::cout << "~TCPSession" << std::endl;
                }

                void Start()
                {
                    m_subscriber = m_factory.Create(std::weak_ptr<IControl>(shared_from_this()),
                        std::weak_ptr<IPublisher>(shared_from_this()));

                    DoRead();
                }

            private:
                boost::asio::ip::tcp::socket m_socket;
                Common::ThreadPool &m_workers;
                ISubscriberFactory &m_factory;
                std::shared_ptr<ISubscriber> m_subscriber;

                //----------------------------------------------------------------------------
                // IPublisher
                virtual void Publish(Buffer buffer) override
                {
                    try
                    {
                        auto self(shared_from_this());

                        m_socket.get_io_service().post([self, buffer] ()
                                {
                                    boost::asio::async_write(self->m_socket, boost::asio::buffer(buffer.second.get(), buffer.first),
                                            [self, buffer] (boost::system::error_code error, std::size_t /*length*/)
                                            {
                                                if (error)
                                                {
                                                    // TODO: error to log
                                                    self->CloseMe();
                                                    std::cout << "OnWrite error." << std::endl;
                                                }
                                            }
                                        );
                                }
                            );
                    }
                    catch (std::exception const &)
                    {
                        // TODO: to log
                        CloseMe();
                    }
                }

                //----------------------------------------------------------------------------
                // IControl
                virtual void CloseMe() override
                {
                    auto self = shared_from_this();
                    m_socket.get_io_service().post([self] ()
                            {
                                try
                                {
                                    self->m_socket.close();
                                }
                                catch (std::exception const &e)
                                {
                                    std::cerr << "CloseMe error: " << e.what() << std::endl;
                                    // TODO: to log
                                }
                            }
                        );
                }

                //----------------------------------------------------------------------------

                void DoRead()
                {
                    std::size_t const bytes = 4096; // TODO: from params
                    auto buffer = std::make_pair(bytes, boost::shared_array<char>(new char [bytes]));
                    auto self(shared_from_this());
                    m_socket.async_read_some(boost::asio::buffer(buffer.second.get(), buffer.first),
                        [self, buffer] (boost::system::error_code error, std::size_t length)
                        {
                            try
                            {
                                if (!error)
                                {
                                    self->m_workers.Post([self, length, buffer] ()
                                            {
                                                try
                                                {
                                                    self->m_subscriber->OnData(std::make_pair(length, std::move(buffer.second)));
                                                }
                                                catch (std::exception const &e)
                                                {
                                                    // TODO: to log
                                                    std::cerr << "Failed to process data. Error: " << e.what() << std::endl;
                                                }
                                            }
                                        );
                                    self->DoRead();
                                }
                                else
                                {
                                    // TODO: to log
                                    self->CloseMe();
                                    //std::cerr << "Failed to process data." << std::endl;
                                }
                            }
                            catch (std::exception const &e)
                            {
                                // TODO: to lgg
                                self->CloseMe();
                                std::cerr << "Failed to process data. Error: " << e.what() << std::endl;
                            }
                        }
                    );
                }
            };

            class TCPServer final
            {
            public:
                TCPServer(std::string const &host, std::string const &port,
                    std::uint16_t workers, std::shared_ptr<ISubscriberFactory> factory)
                try
                    : m_factory{factory}
                    , m_workers{std::make_shared<Common::ThreadPool>(workers)}
                    , m_acceptor{m_ioService,
                            [this, &host, &port] () -> boost::asio::ip::tcp::endpoint
                            {
                                boost::asio::ip::tcp::resolver resolver{m_ioService};
                                boost::asio::ip::tcp::resolver::query query{host, port};
                                return *resolver.resolve(query);
                            } ()
                        }
                    , m_socket{m_ioService}
                    , m_work{m_ioService}
                {
                    std::exception_ptr exception{};
                    m_thread.reset(new std::thread([this, &exception] ()
                            {
                                try
                                {
                                    m_ioService.run();
                                }
                                catch (std::exception const &e)
                                {
                                    exception = std::current_exception();
                                }
                            }
                        )
                    );

                    if (exception)
                        std::rethrow_exception(exception);

                    DoAccept();
                }
                catch (std::exception const &e)
                {
                    throw std::runtime_error{"[Mif::Net::Detail::TCPServer] Failed to start server. "
                        "Error: " + std::string{e.what()}};
                }
                catch (...)
                {
                    throw std::runtime_error{"[Mif::Net::Detail::TCPServer] Failed to start server. Error: unknown."};
                }

                virtual ~TCPServer()
                {
                    try
                    {
                        m_ioService.post([this] ()
                                {
                                    try
                                    {
                                        m_ioService.stop();
                                    }
                                    catch (std::exception const &e)
                                    {
                                        std::cerr << "[Mif::Net::Detail::~TCPServer] Failed to post 'stop' to server object. Error: " << e.what() << std::endl;
                                    }
                                }
                            );

                        m_thread->join();
                    }
                    catch (std::exception const &e)
                    {
                        std::cerr << "[Mif::Net::Detail::~TCPServer] Failed to stop server. Error: " << e.what() << std::endl;
                    }
                }

            private:
                std::shared_ptr<ISubscriberFactory> m_factory;
                std::shared_ptr<Common::ThreadPool> m_workers;
                std::unique_ptr<std::thread> m_thread;
                boost::asio::io_service m_ioService;
                boost::asio::ip::tcp::acceptor m_acceptor;
                boost::asio::ip::tcp::socket m_socket;
                boost::asio::io_service::work m_work;

                void DoAccept()
                {
                    m_acceptor.async_accept(m_socket,
                            [this] (boost::system::error_code error)
                            {
                                try
                                {
                                    if (!error)
                                        std::make_shared<TCPSession>(std::move(m_socket), *m_workers, *m_factory)->Start();
                                    else
                                    {
                                        std::cerr << "[Mif::Net::Detail::TCPServer::DoAccept] Failed tp accept connection." << std::endl;
                                    }
                                    DoAccept();
                                }
                                catch (std::exception const &e)
                                {
                                    std::cerr << "[Mif::Net::Detail::TCPServer::DoAccept] Failed to accept connection. Error: " << e.what() << std::endl;
                                }
                            }
                        );
                }
            };

            class TCPClients final
            {
            public:
                TCPClients()
                {
                    ;
                }

                ~TCPClients()
                {
                    ;
                }

            private:
            };

        }   // namespace Detail
    }   // namespace Net
}   // namespace Mif

int main()
{
    try
    {
        auto server = std::make_shared<Mif::Net::Detail::TCPServer>(
            "localhost", "5555", 4, std::make_shared<Mif::Net::ClientFactory>());
        std::cin.get();
    }
    catch (std::exception const &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}
