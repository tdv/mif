// MIF
#include <mif/application/application.h>
#include <mif/common/log.h>

class Application
    : public Mif::Application::Application
{
public:
    Application(int argc, char const **argv)
        : Mif::Application::Application{argc, argv}
    {
        MIF_LOG(Info) << "Application";
    }
    
    ~Application()
    {
        MIF_LOG(Info) << "~Application";
    }
    
    virtual void OnStart() override final
    {
        MIF_LOG(Info) << "OnStart";
    }

    virtual void OnStop() override final
    {
        MIF_LOG(Info) << "OnStop";
    }
};

int main(int argc, char const **argv)
{
    return Mif::Application::Run<Application>(argc, argv);
}
