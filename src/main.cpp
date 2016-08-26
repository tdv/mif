#include <iostream>
#include <functional>
#include <memory>
#include <vector>

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
    namespace Net
    {

        using Buffer = std::pair<std::size_t, boost::shared_array<char>>;

        struct IControl
        {
            virtual ~IControl() = default;
            virtual void CloseMe() = 0;
        };

        using IControlPtr = std::shared_ptr<IControl>;

        struct IPublisher
        {
            virtual ~IPublisher() = default;
            virtual void Publish(Buffer buffer) = 0;
        };

        using IPublisherPtr = std::shared_ptr<IPublisher>;

        struct ISubscriber
        {
            virtual ~ISubscriber() = default;
            virtual void OnData(Buffer buffer) = 0;
        };

        using ISubscriberPtr = std::shared_ptr<ISubscriber>;

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
                throw std::runtime_error{"OnData not implemented."};
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
                TCPSession(boost::asio::ip::tcp::socket socket, ISubscriberFactory &factory)
                    : m_socket(std::move(socket))
                    , m_factory(factory)
                {
                }

                void Start()
                {
                    m_subscriber = m_factory.Create(std::weak_ptr<IControl>(shared_from_this()),
                        std::weak_ptr<IPublisher>(shared_from_this()));

                    DoRead();
                }

            private:
                boost::asio::ip::tcp::socket m_socket;
                ISubscriberFactory &m_factory;
                std::shared_ptr<ISubscriber> m_subscriber;

                //----------------------------------------------------------------------------
                // IPublisher
                virtual void Publish(Buffer buffer) override
                {
                    try
                    {
                        auto self(shared_from_this());

                        auto publisherTask = [self, buffer] ()
                        {
                            boost::asio::async_write(self->m_socket, boost::asio::buffer(buffer.second.get(), buffer.first),
                                    [self, buffer] (boost::system::error_code error, std::size_t /*length*/)
                                    {
                                        if (error)
                                        {
                                            // TODO: error to log
                                            self->CloseMe();
                                        }
                                    }
                                );
                        };

                        m_socket.get_io_service().post(publisherTask);
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
                    try
                    {
                        auto self = shared_from_this();
                        m_socket.get_io_service().post([self] () { self->m_socket.close(); } );
                    }
                    catch (std::exception const &)
                    {
                        // TODO: to log
                    }
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
                                    self->m_subscriber->OnData(std::move(buffer));
                                else
                                    self->CloseMe();
                            }
                            catch (std::exception const &)
                            {
                                // TODO: to lgg
                                self->CloseMe();
                            }
                        }
                    );
                }
            };

            class TCPServer final
            {
            public:
                TCPServer(std::string const &host, std::uint16_t port,
                          std::uint16_t ioTreadCount,
                          std::shared_ptr<ISubscriberFactory> factory)
                    : m_factory(factory)
                    , m_acceptor(m_ioService, boost::asio::ip::tcp::endpoint(
                            boost::asio::ip::address::from_string(host), port)
                        )
                    , m_socket(m_ioService)
                {
                    std::unique_ptr<boost::asio::io_service::work> ioWork(new boost::asio::io_service::work(m_ioService));
                    for (std::uint16_t i = 0 ; i < ioTreadCount ; ++i)
                    {
                        std::exception_ptr exception{};
                        m_ioTreads.create_thread([this, &exception] ()
                                {
                                    try
                                    {
                                        m_ioService.run();
                                    }
                                    catch (std::exception const &)
                                    {
                                        // TODO: to log
                                        exception = std::current_exception();
                                    }
                                }
                            );
                        if (exception)
                        {
                            m_socket.close();
                            ioWork.reset();
                            m_ioTreads.join_all();
                            std::rethrow_exception(exception);
                        }
                        m_ioWork = std::move(ioWork);

                        DoAccept();
                    }
                }

                virtual ~TCPServer()
                {
                    try
                    {
                        m_socket.close();
                        m_ioWork.reset();
                        m_ioTreads.join_all();
                    }
                    catch (std::exception const &)
                    {
                        // TODO: to log
                    }
                }

            private:
                std::shared_ptr<ISubscriberFactory> m_factory;
                boost::asio::io_service m_ioService;
                std::unique_ptr<boost::asio::io_service::work> m_ioWork;
                boost::asio::ip::tcp::acceptor m_acceptor;
                boost::asio::ip::tcp::socket m_socket;
                boost::thread_group m_ioTreads;

                void DoAccept()
                {
                    m_acceptor.async_accept(m_socket,
                            [this] (boost::system::error_code error)
                            {
                                if (!error)
                                {
                                    std::make_shared<TCPSession>(std::move(m_socket), *m_factory)->Start();
                                }
                                DoAccept();
                            }
                        );
                }
            };
        }   // namespace Detail
    }   // namespace Net
}   // namespace Mif

int main()
{
    try
    {
        auto server = std::make_shared<Mif::Net::Detail::TCPServer>(
            "127.0.0.1", 5555, 2, std::make_shared<Mif::Net::ClientFactory>());
        std::cin.get();
    }
    catch (std::exception const &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}
