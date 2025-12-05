#include "app/application.hpp"
#include "renderer/renderer.hpp"
#include <utils/logging.hpp>

namespace ankh
{

    void Application::run()
    {
        ANKH_LOG_INFO("Starting application");
        Renderer renderer;
        renderer.run();
        ANKH_LOG_INFO("Shutting down application");
    }

} // namespace ankh
