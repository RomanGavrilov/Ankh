#include "app/application.hpp"
#include "renderer/renderer.hpp"
#include <utils/logging.hpp>

namespace ankh
{

    void Application::run()
    {
        ANKH_LOG_DEBUG("[App] Starting application");
        Renderer renderer;
        renderer.run();
        ANKH_LOG_DEBUG("[App] Shutting down application");
    }

} // namespace ankh
