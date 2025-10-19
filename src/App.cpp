#include "App.hpp"

#include "Util/Logging.hpp"

#include "Renderer/Frustum.hpp"

#include <glad/glad.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <unordered_map>

namespace
{
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    auto* app = static_cast<App*>(glfwGetWindowUserPointer(window));
    if (!app)
        return;
    glViewport(0, 0, width, height);
    app->on_resize(width, height);
}
}

App::App() = default;

App::~App()
{
    glfwTerminate();
}

bool App::initialize()
{
    static bool glfwInitialized = false;
    if (!glfwInitialized)
    {
        if (!glfwInit())
        {
            util::log().error("Failed to initialize GLFW");
            return false;
        }
        glfwInitialized = true;
    }

    const auto settings = config::app();

    if (!m_context.window())
    {
        m_context = renderer::GLContext::create(settings.windowSize.x, settings.windowSize.y, settings.startFullscreen, "CodexCraft");
        if (!m_context.window())
        {
            return false;
        }

        glfwSetWindowUserPointer(m_context.window(), this);
        glfwSetInputMode(m_context.window(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        glfwSetFramebufferSizeCallback(m_context.window(), framebuffer_size_callback);
    }

    const auto atlasSettings = config::atlas();
    if (!m_atlas.load_atlas("assets/atlas.png", atlasSettings.tilesX, atlasSettings.tilesY))
    {
        util::log().error("Failed to load texture atlas. Place atlas.png in assets/");
        return false;
    }

    if (!m_chunkShader.load("shaders/chunk.vert", "shaders/chunk.frag"))
    {
        util::log().error("Failed to load shaders");
        return false;
    }

    m_chunkShader.use();
    m_chunkShader.set_int("uAtlas", 0);

    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);

    m_camera.set_position(glm::vec3(0.0f, 90.0f, 0.0f));
    m_camera.set_rotation(glm::radians(-20.0f), 0.0f);
    m_camera.set_perspective(glm::radians(70.0f), static_cast<float>(settings.windowSize.x) / settings.windowSize.y, 0.1f, 1000.0f);

    m_frameTimer.reset();

    return true;
}

void App::run()
{
    if (!m_context.window())
    {
        if (!initialize())
            return;
    }

    auto* window = m_context.window();
    while (m_running && !glfwWindowShouldClose(window))
    {
        const double dt = m_frameTimer.elapsed_seconds();
        m_frameTimer.reset();

        glfwPollEvents();
        handle_input(static_cast<float>(dt));
        update(static_cast<float>(dt));
        render();

        glfwSwapBuffers(window);
    }
}

void App::on_resize(int width, int height)
{
    if (height == 0)
        return;
    m_camera.resize(width, height);
}

void App::handle_input(float dt)
{
    auto* window = m_context.window();
    const float moveSpeed = 40.0f;
    const float fastMultiplier = 3.0f;
    float speed = moveSpeed;
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
    {
        speed *= fastMultiplier;
    }

    glm::vec3 move{0.0f};
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        move += m_camera.forward();
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        move -= m_camera.forward();

    const glm::vec3 right = glm::normalize(glm::cross(m_camera.forward(), glm::vec3(0.0f, 1.0f, 0.0f)));
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        move += right;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        move -= right;
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        move.y += 1.0f;
    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
        move.y -= 1.0f;

    if (glm::length(move) > 0.0f)
    {
        move = glm::normalize(move);
        m_camera.move(move * speed * dt);
    }

    double xpos = 0.0, ypos = 0.0;
    glfwGetCursorPos(window, &xpos, &ypos);
    glm::vec2 currentMouse(static_cast<float>(xpos), static_cast<float>(ypos));
    if (m_firstMouse)
    {
        m_lastMouse = currentMouse;
        m_firstMouse = false;
    }

    const glm::vec2 delta = currentMouse - m_lastMouse;
    m_lastMouse = currentMouse;
    const float sensitivity = 0.0025f;
    m_camera.add_rotation(delta.y * -sensitivity, delta.x * -sensitivity);

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        m_running = false;
    }

    auto handle_toggle = [&](int key, auto onPress) {
        static std::unordered_map<int, bool> previous;
        const bool pressed = glfwGetKey(window, key) == GLFW_PRESS;
        bool& prev = previous[key];
        if (pressed && !prev)
        {
            onPress();
        }
        prev = pressed;
    };

    handle_toggle(GLFW_KEY_F1, [this] { toggle_wireframe(); });
    handle_toggle(GLFW_KEY_F2, [this] { reload_shaders(); });
    handle_toggle(GLFW_KEY_F3, [this] {
        const auto stats = m_streamer.stats();
        util::log().info("Chunks: total=%zu generating=%zu meshPending=%zu uploaded=%zu meshing=%zu pendingUploads=%zu | jobs gen=%zu mesh=%zu",
                         stats.totalChunks,
                         stats.generating,
                         stats.meshPending,
                         stats.uploaded,
                         stats.meshing,
                         stats.pendingUploads,
                         m_streamer.pending_generation_jobs(),
                         m_streamer.pending_meshing_jobs());
    });
}

void App::update(float dt)
{
    (void)dt;
    m_streamer.update(m_camera.position());
}

void App::render()
{
    glClearColor(0.52f, 0.78f, 0.96f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    renderer::Frustum frustum;
    frustum.update(m_camera.projection() * m_camera.view());

    std::vector<world::DrawCommand> opaque;
    std::vector<world::DrawCommand> transparent;
    m_streamer.gather_draw_commands(m_camera, frustum, opaque, transparent);

    m_chunkShader.use();
    m_chunkShader.set_mat4("uProjection", m_camera.projection());
    m_chunkShader.set_mat4("uView", m_camera.view());
    m_chunkShader.set_vec3("uLightDir", glm::normalize(glm::vec3(-0.3f, -1.0f, -0.2f)));
    m_atlas.bind(0);

    glPolygonMode(GL_FRONT_AND_BACK, m_wireframe ? GL_LINE : GL_FILL);
    glDisable(GL_BLEND);

    for (const auto& cmd : opaque)
    {
        const glm::mat4 model = glm::translate(glm::mat4(1.0f), cmd.chunk->world_position());
        m_chunkShader.set_mat4("uModel", model);
        cmd.mesh->draw_opaque(cmd.lod);
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    for (const auto& cmd : transparent)
    {
        const glm::mat4 model = glm::translate(glm::mat4(1.0f), cmd.chunk->world_position());
        m_chunkShader.set_mat4("uModel", model);
        cmd.mesh->draw_transparent(cmd.lod);
    }
    glDisable(GL_BLEND);
}

void App::toggle_wireframe()
{
    m_wireframe = !m_wireframe;
}

void App::reload_shaders()
{
    m_chunkShader.reload();
    m_chunkShader.use();
    m_chunkShader.set_int("uAtlas", 0);
    m_streamer.reload();
}
