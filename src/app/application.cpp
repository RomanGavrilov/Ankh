#include "app/application.hpp"
#include "renderer/renderer.hpp"

namespace ankh
{

    void Application::run()
    {
        Renderer renderer;
        renderer.run();
    }

} // namespace ankh
