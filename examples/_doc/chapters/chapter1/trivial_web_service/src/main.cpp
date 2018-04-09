// STD
#include <map>
#include <mutex>
#include <stdexcept>
#include <string>

// MIF
#include <mif/application/http_server.h>
#include <mif/common/static_string.h>
#include <mif/common/types.h>
#include <mif/common/unused.h>
#include <mif/net/http/constants.h>

class Application
    : public Mif::Application::HttpServer
{
public:
    using HttpServer::HttpServer;

private:
    using KeyParamId = MIF_STATIC_STR("key");

    using LockType = std::mutex;
    using LockGuard = std::lock_guard<LockType>;
    using Cache = std::map<std::string, Mif::Common::Buffer>;

    LockType m_lock;
    Cache m_cache;

    std::string GetKey(Mif::Net::Http::IInputPack::Params const &params) const
    {
        auto const key = params.find(KeyParamId::Value);
        if (key == std::end(params))
            throw std::invalid_argument{"No key"};
        return key->second;
    }

    virtual void Init(Mif::Net::Http::ServerHandlers &handlers) override final
    {
        handlers["/set"] = std::bind(&Application::Set, this, std::placeholders::_1, std::placeholders::_2);
        handlers["/get"] = std::bind(&Application::Get, this, std::placeholders::_1, std::placeholders::_2);
        handlers["/del"] = std::bind(&Application::Del, this, std::placeholders::_1, std::placeholders::_2);
        handlers["/clean"] = std::bind(&Application::Clean, this, std::placeholders::_1, std::placeholders::_2);
    }

    void Set(Mif::Net::Http::IInputPack const &req, Mif::Net::Http::IOutputPack &resp)
    {
        Mif::Common::Unused(resp);

        auto const params = req.GetParams();
        auto const key = GetKey(params);

        auto data = req.GetData();

        LockGuard lock{m_lock};
        m_cache[key] = std::move(data);
    }

    void Get(Mif::Net::Http::IInputPack const &req, Mif::Net::Http::IOutputPack &resp)
    {
        auto const params = req.GetParams();
        auto const key = GetKey(params);

        {
            LockGuard lock{m_lock};
            auto const iter = m_cache.find(key);
            if (iter == std::end(m_cache))
                throw std::invalid_argument{"Key \"" + key + "\" not found."};
            resp.SetData(iter->second);
        }

        resp.SetHeader(Mif::Net::Http::Constants::Header::Response::ContentType::Value,
                "text/html; charset=UTF-8");
    }

    void Del(Mif::Net::Http::IInputPack const &req, Mif::Net::Http::IOutputPack &resp)
    {
        Mif::Common::Unused(resp);

        auto const params = req.GetParams();
        auto const key = GetKey(params);

        LockGuard lock{m_lock};
        auto const iter = m_cache.find(key);
        if (iter == std::end(m_cache))
            throw std::invalid_argument{"Key \"" + key + "\" not found."};
        m_cache.erase(iter);
    }

    void Clean(Mif::Net::Http::IInputPack const &req, Mif::Net::Http::IOutputPack &resp)
    {
        Mif::Common::Unused(req);
        Mif::Common::Unused(resp);

        LockGuard lock{m_lock};
        m_cache.clear();
    }
};

int main(int argc, char const **argv)
{
    return Mif::Application::Run<Application>(argc, argv);
}
