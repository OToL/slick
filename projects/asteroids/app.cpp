#include "app.hpp"
#include "graphics.hpp"
#include "cannon.hpp"
#include "projectile.hpp"
#include "ennemy.hpp"

#include <slk/core.hpp>
#include <slk/color.hpp>
#include <slk/math/math.hpp>
#include <slk/math/matrix2.hpp>
#include <slk/math/matrix4.hpp>
#include <slk/math/graphics.hpp>
#include <slk/math/utils.hpp>
#include <slk/input/input_api.hpp>
#include <slk/input/events.hpp>

#include <sokol/sokol_app.h>

#include <bgfx/bgfx.h>
#include <bgfx_utils/debugdraw/debugdraw.h>

#include <chrono>
#include <vector>
#include <random>
#include <cassert>

using namespace slk::literals;

const slk::f32 ENNEMY_SPEED = 0.4f;
const slk::AABB2f NEW_ENEMY_EXTENTS{
    slk::Vector2f(-64, -64),
    slk::Vector2f(64, 64),
};
const slk::AABB2f ENV_AABB = {slk::Vector2f(200.f, 70.f), slk::Vector2f(1000.f, 670.f)};
const slk::f32 CANNON_SIZE = 50.f;
const slk::f32 CANNON_MAX_ANGLE = 70.0_deg;
const slk::f32 CANNON_SHOOT_SPEED = 120;
const slk::f32 PROJECTILE_SPEED = .5f;

constexpr slk::u32 RAND_SEED = 42;
std::mt19937 pseudo_rndr(RAND_SEED);
std::uniform_real_distribution dist_rndr(0.f, 1.f); // [0, 1[

template <typename T>
void removeSwap(std::vector<T> & vec, typename std::vector<T>::iterator rem_it) {
    assert(rem_it >= vec.begin() && rem_it < vec.end());

    // guard against self swap/move because some types may not handle it properly
    if (rem_it != (vec.end() - 1)) {
        std::swap(vec.back(), *rem_it);
    }

    vec.resize(vec.size() - 1);
}

slk::f32 getRandom() {
    return dist_rndr(pseudo_rndr);
}

void resetRandom() {
    pseudo_rndr = std::mt19937(RAND_SEED);
}

slk::Vector2f get_rand_pos_in_abb(slk::AABB2f const & aabb) {
    return aabb.m_min + slk::Vector2f{
                          static_cast<slk::f32>(getRandom()) * aabb.width(),
                          static_cast<slk::f32>(getRandom()) * aabb.height(),
                      };
}

struct GameState {
    const slk::AABB2f env_aabb = ENV_AABB;

    std::vector<Ennemy> ennemies;
    std::vector<Projectile> projectiles;
    Cannon cannon = {ENV_AABB.m_min + slk::Vector2f(ENV_AABB.width() * 0.5f, 0) + slk::Vector2f(0, 50),
                     CANNON_SIZE,
                     CANNON_SIZE,
                     CANNON_MAX_ANGLE,
                     CANNON_SHOOT_SPEED,
                     PROJECTILE_SPEED};

    void reset() {
        resetRandom();

        ennemies.clear();
        projectiles.clear();
        cannon.reset();

        ennemies.reserve(255);
        projectiles.reserve(255);

        const slk::Vector2f SPAWN_MARGIN = slk::Vector2f{5.f, 5.f};

        // define the sub area where ennemies must be spawned
        slk::Vector2f const spawn_margin = NEW_ENEMY_EXTENTS.m_max + SPAWN_MARGIN;
        slk::AABB2f env_spawn_space{slk::Vector2f(env_aabb.m_min + slk::Vector2f(0, env_aabb.height() * 0.444f)),
                                    env_aabb.m_max - slk::Vector2f(env_aabb.width() * .5f, 0)};
        env_spawn_space.shrink(spawn_margin);

        slk::f32 const ennemy1_ang = getRandom() * slk::PI_F32 * 2;
        slk::Vector2f ennemy1_pos = get_rand_pos_in_abb(env_spawn_space);
        ennemies.emplace_back(ennemy1_pos, ennemy1_pos, slk::Vector2f(std::cos(ennemy1_ang), std::sin(ennemy1_ang)) * ENNEMY_SPEED,
                              NEW_ENEMY_EXTENTS);

        slk::f32 const ennemy2_ang = getRandom() * slk::PI_F32 * 2;
        slk::Vector2f ennemy2_pos = get_rand_pos_in_abb(env_spawn_space) + slk::Vector2f(ENV_AABB.width() * .5f, 0.f);
        ennemies.emplace_back(ennemy2_pos, ennemy2_pos, slk::Vector2f(std::cos(ennemy2_ang), std::sin(ennemy2_ang)) * ENNEMY_SPEED,
                              NEW_ENEMY_EXTENTS);
    }
};

// Logical resolution: game coordinates are in pixels, origin bottom-left, y up
constexpr slk::f32 WORLD_WIDTH = 1200.f;
constexpr slk::f32 WORLD_HEIGHT = 720.f;
constexpr bgfx::ViewId MAIN_VIEW_ID = 0;
constexpr slk::f32 CANNON_ROT_SPEED = 0.005f;

// slk::InputApi only exposes key down states (no pressed/released yet)
class KeyReleaseTracker {
public:
    explicit KeyReleaseTracker(slk::KeyboardVKey key) : m_key(key) { }

    // Must be called once per frame
    slk::b8 update(slk::InputApi const& input) {
        slk::b8 const is_down = input.isKeyboardKeyDown(m_key);
        slk::b8 const released = m_was_down && !is_down;
        m_was_down = is_down;

        return released;
    }

private:
    slk::KeyboardVKey m_key;
    slk::b8 m_was_down = false;
};

struct AppState {
    using Timer = std::chrono::high_resolution_clock;

    GameState game_state;
    std::vector<Ennemy> new_ennemies;

    KeyReleaseTracker reset_key{slk::KeyboardVKey::CHAR_R};
    KeyReleaseTracker shoot_key{slk::KeyboardVKey::SPACE};

    Timer::time_point last_frame_time_point;
    slk::f32 delta_time_ms = 0.f;
};

AppState* g_app_state = nullptr;

void updateGame(AppState& app_state, slk::InputApi const& input) {
    GameState& game_state = app_state.game_state;
    std::vector<Ennemy>& new_ennemies = app_state.new_ennemies;
    slk::f32 const delta_time_ms = app_state.delta_time_ms;

    game_state.cannon.update(delta_time_ms);

    slk::b8 const reset_released = app_state.reset_key.update(input);
    slk::b8 const shoot_released = app_state.shoot_key.update(input);

    if (game_state.ennemies.empty() || reset_released) {
        game_state.reset();
    }
    if (input.isAnyKeyboardKeyDown(std::array{slk::KeyboardVKey::CHAR_J, slk::KeyboardVKey::LEFT})) {
        game_state.cannon.turn(CANNON_ROT_SPEED * delta_time_ms);
    }
    if (input.isAnyKeyboardKeyDown(std::array{slk::KeyboardVKey::CHAR_L, slk::KeyboardVKey::RIGHT})) {
        game_state.cannon.turn(-CANNON_ROT_SPEED * delta_time_ms);
    }
    if (shoot_released) {
        if (auto new_projectile = game_state.cannon.shoot()) {
            game_state.projectiles.push_back(*new_projectile);
        }
    }

    // projectiles integration + environment collision
    for (auto proj_iter = begin(game_state.projectiles); proj_iter != end(game_state.projectiles);) {
        Projectile & proj = *proj_iter;

        proj.prev_pos = proj.curr_pos;
        proj.curr_pos = proj.prev_pos + proj.velocity * delta_time_ms;

        // out of environment bound
        if (!game_state.env_aabb.isInside(proj.curr_pos)) {
            removeSwap(game_state.projectiles, proj_iter);
        } else {
            ++proj_iter;
        }
    }

    // ennemies integration + environment collisions
    for (auto & ennemy : game_state.ennemies) {
        const slk::Vector2f move = ennemy.velocity * delta_time_ms;

        ennemy.prev_position = ennemy.position;
        ennemy.position += move;
        auto ennemy_world_aabb = ennemy.extents.displaced(ennemy.position);

        // the ennemy is exiting the environment
        if (!game_state.env_aabb.isInside(ennemy_world_aabb)) {
            slk::Vector2f normal = slk::Vector2f::ZERO;

            // simple snapping
#if 1
            const slk::Vector2f plane_normals[] = {
                slk::Vector2f{-1.f, 0.f},
                slk::Vector2f{0.f, -1.f},
                slk::Vector2f{1.f, 0.f},
                slk::Vector2f{0.f, 1.f},
            };

            auto diff_max = slk::max(ennemy_world_aabb.m_max - game_state.env_aabb.m_max, slk::Vector2f::zero());
            ennemy.position -= diff_max;

            for (slk::u32 i = 0; i != 2; ++i) {
                normal += plane_normals[i] * (float)(diff_max.m_values[i] > 0);
            }

            auto diff_min = slk::min(ennemy_world_aabb.m_min - game_state.env_aabb.m_min, slk::Vector2f::zero());
            ennemy.position -= diff_min;

            for (slk::u32 i = 2; i != 4; ++i) {
                normal += plane_normals[i] * (float)(diff_min.m_values[i & 1] < 0);
            }

            // plane equation
#else
            struct Plane {
                slk::Vector2f normal;
                slk::f32 d;
            };

            const Plane planes[] = {
                {slk::Vector2f{0.f, -1.f}, game_state.env_aabb.max.y},
                {slk::Vector2f{-1.f, 0.f}, game_state.env_aabb.max.x},
                {slk::Vector2f{1.f, 0.f}, -game_state.env_aabb.min.x},
                {slk::Vector2f{0.f, 1.f}, -game_state.env_aabb.min.y},
            };

            const auto plane_test = [](Plane const & plane, slk::Vector2f const & point) {
                auto res = point.dot(plane.normal) + plane.d;
                return res;
            };

            // test ennemy's aaabb max against top and right environment planes and snap it if behind
            for (int i = 0; i != 2; ++i) {
                Plane const & plane = planes[i];
                if (const slk::f32 res = plane_test(plane, world_aabb.max); res <= 0) {
                    ennemy.position += plane.normal * -res;
                    world_aabb = ennemy.extents.displaced(ennemy.position);
                    normal += plane.normal;
                }
            }
            // test ennemy's aaabb min against bottom and left environment planes and snap it if behind
            for (int i = 2; i != 4; ++i) {
                Plane const & plane = planes[i];
                if (const auto res = plane_test(plane, world_aabb.min); res <= 0) {
                    ennemy.position += plane.normal * -res;
                    world_aabb = ennemy.extents.displaced(ennemy.position);
                    normal += plane.normal;
                }
            }
#endif

            // reflect ennemeny's velocity against collision planes' normal
            normal.normalize();
            const auto vel_proj = ennemy.velocity.dot(normal);
            ennemy.velocity = normal * (-2 * vel_proj) + ennemy.velocity;
        }
    }

    // ennemy/ennemy collisions
    if (!game_state.ennemies.empty()) {
        for (auto ennemy_iter = begin(game_state.ennemies); ennemy_iter != end(game_state.ennemies) - 1; ennemy_iter++) {
            slk::AABB2f world_ennemy1_aabb = ennemy_iter->extents.displaced(ennemy_iter->position);
            for (auto ennemy_iter2 = ennemy_iter + 1; ennemy_iter2 != end(game_state.ennemies); ++ennemy_iter2) {
                slk::AABB2f world_ennemy2_aabb = ennemy_iter2->extents.displaced(ennemy_iter2->position);
                if (world_ennemy1_aabb.overlaps(world_ennemy2_aabb)) {
                    const slk::AABB2f prev_world_ennemy1_aabb = ennemy_iter->extents.displaced(ennemy_iter->prev_position);
                    const slk::AABB2f prev_world_ennemy2_aabb = ennemy_iter2->extents.displaced(ennemy_iter2->prev_position);
                    slk::Vector2f normal = slk::Vector2f::zero();

                    // find-out first ennemy incoming side with respect to the other one to ...
                    // 1. deduce the collision normal
                    // 2. cancel out the penetration (only on first ennemy to simplify)

                    if (prev_world_ennemy1_aabb.m_min.m_x >= prev_world_ennemy2_aabb.m_max.m_x && world_ennemy1_aabb.m_min.m_x < world_ennemy2_aabb.m_max.m_x) {
                        ennemy_iter->position.m_x += (world_ennemy2_aabb.m_max.m_x - world_ennemy1_aabb.m_min.m_x) + std::numeric_limits<float>::epsilon();
                        normal += slk::Vector2f(1, 0);
                    }

                    if (prev_world_ennemy1_aabb.m_min.m_y >= prev_world_ennemy2_aabb.m_max.m_y && world_ennemy1_aabb.m_min.m_y < world_ennemy2_aabb.m_max.m_y) {
                        ennemy_iter->position.m_y += (world_ennemy2_aabb.m_max.m_y - world_ennemy1_aabb.m_min.m_y) + std::numeric_limits<float>::epsilon();
                        normal += slk::Vector2f(0, 1);
                    }

                    if (prev_world_ennemy1_aabb.m_max.m_x <= prev_world_ennemy2_aabb.m_min.m_x && world_ennemy1_aabb.m_max.m_x > world_ennemy2_aabb.m_min.m_x) {
                        ennemy_iter->position.m_x += (world_ennemy2_aabb.m_min.m_x - world_ennemy1_aabb.m_max.m_x) - std::numeric_limits<float>::epsilon();
                        normal += slk::Vector2f(-1, 0);
                    }
                    if (prev_world_ennemy1_aabb.m_max.m_y <= prev_world_ennemy2_aabb.m_min.m_y && world_ennemy1_aabb.m_max.m_y > world_ennemy2_aabb.m_min.m_y) {
                        ennemy_iter->position.m_y += (world_ennemy2_aabb.m_min.m_y - world_ennemy1_aabb.m_max.m_y) - std::numeric_limits<float>::epsilon();
                        normal += slk::Vector2f(0, -1);
                    }

                    // this could happen if the frame time is too long and the penetration too important
                    if (normal.m_x == 0 && normal.m_y == 0) {
                        continue;
                    }

                    normal.normalize();
                    if (ennemy_iter->velocity.dot(normal) < 0) {
                        const auto vel_proj = ennemy_iter->velocity.dot(normal);
                        ennemy_iter->velocity = normal * (-2 * vel_proj) + ennemy_iter->velocity;
                    }
                    normal *= -1;
                    if (ennemy_iter2->velocity.dot(normal) < 0) {
                        const auto vel_proj = ennemy_iter2->velocity.dot(normal);
                        ennemy_iter2->velocity = normal * (-2 * vel_proj) + ennemy_iter2->velocity;
                    }
                }
            }
        }
    }

    // snap all ennemies to not let them escape after resolution of ennemy/ennemy collisions
    for (auto & ennemy : game_state.ennemies) {
        slk::AABB2f ennemy_aabb = ennemy.extents.displaced(ennemy.position);

        auto diff_max = slk::max(ennemy_aabb.m_max - game_state.env_aabb.m_max, slk::Vector2f::zero());
        ennemy.position -= diff_max;

        auto diff_min = slk::min(ennemy_aabb.m_min - game_state.env_aabb.m_min, slk::Vector2f::zero());
        ennemy.position -= diff_min;
    }

    // projectile/ennemy collisions
    for (auto proj_iter = begin(game_state.projectiles); proj_iter != end(game_state.projectiles);) {
        slk::b8 has_hit = false;
        for (auto ennemy_iter = begin(game_state.ennemies); ennemy_iter != end(game_state.ennemies); ++ennemy_iter) {
            slk::AABB2f world_aabb = ennemy_iter->extents.displaced(ennemy_iter->position);
            if (world_aabb.isInside(proj_iter->curr_pos)) {
                const slk::f32 curr_width = ennemy_iter->extents.width();
                if (curr_width >= 32) {
                    const slk::AABB2f new_extents = ennemy_iter->extents * 0.5f;
                    const slk::Vector2f new_ennemy1 = ennemy_iter->position + new_extents.m_max;
                    const slk::Vector2f new_ennemy2 = ennemy_iter->position + new_extents.m_min;

                    new_ennemies.emplace_back(new_ennemy1, new_ennemy1, ennemy_iter->velocity, new_extents);
                    new_ennemies.emplace_back(new_ennemy2, new_ennemy2, -ennemy_iter->velocity, new_extents);
                }

                removeSwap(game_state.ennemies, ennemy_iter);
                has_hit = true;

                break;
            }
        }

        if (has_hit) {
            removeSwap(game_state.projectiles, proj_iter);
        } else {
            ++proj_iter;
        }
    }

    if (!new_ennemies.empty()) {
        game_state.ennemies.insert_range(game_state.ennemies.end(), new_ennemies);
        new_ennemies.clear();
    }
}

void renderGame(GameState const& game_state, DebugDrawEncoder& dde) {
    constexpr slk::ColorU32 BLACK = slk::ColorU32::black();

    // Draw environment
    draw_aabb(dde, ENV_AABB, slk::Vector2f::zero(), BLACK);

    // Draw canon
    draw_triangle(dde, game_state.cannon.getPos(), game_state.cannon.getRotation(), game_state.cannon.getWidth(),
                  game_state.cannon.getHeight(), BLACK);

    // Compute ennemies' clock rotations
    auto const wall_clock_time = std::chrono::system_clock::now();
    auto const time = std::chrono::system_clock::to_time_t(wall_clock_time);
    auto const actual_time = std::localtime(&time);

    const slk::f32 curr_hour_angle = -((actual_time->tm_hour % 12) / 12.f) * 2 * slk::PI_F32;
    const slk::f32 curr_min_angle = -(actual_time->tm_min / 60.f) * 2 * slk::PI_F32;
    const slk::f32 curr_sec_angle = -(actual_time->tm_sec / 60.f) * 2 * slk::PI_F32;

    const slk::Matrix2f hour_rot = slk::Matrix2f::makeRotation(curr_hour_angle);
    const slk::Matrix2f min_rot = slk::Matrix2f::makeRotation(curr_min_angle);
    const slk::Matrix2f sec_rot = slk::Matrix2f::makeRotation(curr_sec_angle);

    // Render ennemies
    for (auto const & ennemy : game_state.ennemies) {
        draw_aabb(dde, ennemy.extents, ennemy.position, slk::ColorU32::red());

        draw_line(dde, ennemy.position, ennemy.position + hour_rot * slk::Vector2f::unitY() * ennemy.extents.width() * 0.2f, slk::ColorU32::red());
        draw_line(dde, ennemy.position, ennemy.position + min_rot * slk::Vector2f::unitY() * ennemy.extents.width() * 0.4f, slk::ColorU32::green());
        draw_line(dde, ennemy.position, ennemy.position + sec_rot * slk::Vector2f::unitY() * ennemy.extents.width() * 0.3f, slk::ColorU32::blue());
    }

    // Render projectiles
    for (auto const & proj : game_state.projectiles) {
        draw_line(dde, proj.prev_pos, proj.curr_pos, BLACK);
    }
}

void appInitialize() {
    bgfx::Init bgfx_init;
    bgfx_init.type = bgfx::RendererType::Count;
    bgfx_init.resolution.width = sapp_width();
    bgfx_init.resolution.height = sapp_height();
    bgfx_init.resolution.reset = BGFX_RESET_VSYNC;
    bgfx_init.platformData.nwh = appGetWindowHdl();

    bgfx::init(bgfx_init);

    ddInit();

    slk::InputApi::initialize();

    g_app_state = new AppState{};
    g_app_state->game_state.reset();
    g_app_state->new_ennemies.reserve(20);
    g_app_state->last_frame_time_point = AppState::Timer::now();
}

void appUpdate() {
    constexpr slk::u32 CLEAR_COLOR = 0xF5F5F5FF; // RGBA

    slk::InputApi* const input = slk::InputApi::instance();
    input->update();

    if (input->isKeyboardKeyDown(slk::KeyboardVKey::ESCAPE)) {
        sapp_request_quit();
        return;
    }

    auto const time_point = AppState::Timer::now();
    g_app_state->delta_time_ms = std::chrono::duration_cast<std::chrono::microseconds>(time_point - g_app_state->last_frame_time_point).count() / 1000.f;
    g_app_state->last_frame_time_point = time_point;

    updateGame(*g_app_state, *input);

    // orthographic projection mapping the world (in pixels, y up) to the whole window
    slk::Matrix4f const proj = slk::makeOrthoProjectionMatrix(0.f, WORLD_WIDTH, 0.f, WORLD_HEIGHT, -1.f, 1.f, 0.f,
                                                              bgfx::getCaps()->homogeneousDepth, slk::Handedness::DEFAULT);

    bgfx::setViewClear(MAIN_VIEW_ID, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, CLEAR_COLOR, 1.0f, 0);
    bgfx::setViewRect(MAIN_VIEW_ID, 0, 0, sapp_width(), sapp_height());
    bgfx::setViewTransform(MAIN_VIEW_ID, slk::Matrix4f::IDENTITY.data(), proj.data());

    // This dummy draw call is here to make sure that the view is cleared
    // if no other draw calls are submitted to it.
    bgfx::touch(MAIN_VIEW_ID);

    DebugDrawEncoder dde;
    dde.begin(MAIN_VIEW_ID);
    // 2D: no depth test/write, draw order is submission order
    dde.setState(false, false, true);

    renderGame(g_app_state->game_state, dde);

    dde.end();

    bgfx::frame();
}

void appShutdown() {
    delete g_app_state;
    g_app_state = nullptr;

    slk::InputApi::shutdown();

    ddShutdown();

    bgfx::shutdown();
}

void appNotifyEvent(const sapp_event* evt) {
    switch (evt->type) {
        case SAPP_EVENTTYPE_KEY_DOWN: {
            slk::KeyboardEvent slk_event;
            slk_event.m_type = slk::InputEventType::KEYBOARD_KEY_DOWN;
            slk_event.m_key_repeat = evt->key_repeat;
            slk_event.m_vkey = static_cast<slk::KeyboardVKey>(evt->key_code);
            slk::InputApi::instance()->forwardEvent(slk_event);

            break;
        }

        case SAPP_EVENTTYPE_KEY_UP: {
            slk::KeyboardEvent slk_event;
            slk_event.m_type = slk::InputEventType::KEYBOARD_KEY_UP;
            slk_event.m_key_repeat = evt->key_repeat;
            slk_event.m_vkey = static_cast<slk::KeyboardVKey>(evt->key_code);
            slk::InputApi::instance()->forwardEvent(slk_event);

            break;
        }

        // mouse and other events are not used by the game
        default: {
        }
    }
}
