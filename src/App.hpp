#pragma once

#include "Config.hpp"
#include "Core/Timer.hpp"
#include "Renderer/Camera.hpp"
#include "Renderer/Frustum.hpp"
#include "Renderer/GLContext.hpp"
#include "Renderer/Shader.hpp"
#include "Renderer/Texture.hpp"
#include "World/WorldStreamer.hpp"

#include <glm/vec2.hpp>

#include <memory>
#include <vector>

class App
{
  public:
    App();
    ~App();

    bool initialize();
    void run();

    void on_resize(int width, int height);

  private:
    void handle_input(float dt);
    void update(float dt);
    void render();
    void toggle_wireframe();
    void reload_shaders();

    renderer::GLContext m_context;
    renderer::Shader m_chunkShader;
    renderer::Texture m_atlas;

    renderer::Camera m_camera;
    world::WorldStreamer m_streamer;

    glm::vec2 m_lastMouse{0.0f};
    bool m_firstMouse = true;
    bool m_wireframe = false;
    bool m_running = true;

    core::Timer m_frameTimer;
};
