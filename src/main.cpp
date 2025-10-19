#include "App.hpp"
#include "Util/Logging.hpp"

int main()
{
    App app;
    if (!app.initialize())
    {
        util::log().error("Initialization failed");
        return 1;
    }

    app.run();
    return 0;
}
