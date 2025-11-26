#include "app/application.hpp"
#include "renderer/renderer.hpp"

namespace ankh
{

    void Application::run()
    {
        Renderer renderer;
        renderer.run();
    }

    bool Application::run_frames(uint32_t frame_count)
    {
        Renderer renderer;
        return renderer.run_frames(frame_count);
    }

} // namespace ankh
