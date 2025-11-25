#include "graphics.hpp"
#include "cannon.hpp"
#include "projectile.hpp"
#include "ennemy.hpp"
#include "rlib_utils.hpp"

#include <core/core.hpp>
#include <core/color.hpp>
#include <core/math/math.hpp>
#include <core/math/matrix2.hpp>

#include <raylib/raylib.h>

#include <chrono>
#include <vector>
#include <random>
#include <cassert>
#include <deque>

using namespace slk::literals;

// TODO
// - enable sanitizers
// - clang tidy

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
    return aabb.min + slk::Vector2f{
                          static_cast<slk::f32>(getRandom()) * aabb.width(),
                          static_cast<slk::f32>(getRandom()) * aabb.height(),
                      };
}

struct GameState {
    const slk::AABB2f env_aabb = ENV_AABB;

    std::vector<Ennemy> ennemies;
    std::vector<Projectile> projectiles;
    Cannon cannon = {ENV_AABB.min + slk::Vector2f(ENV_AABB.width() * 0.5f, 0) + slk::Vector2f(0, 50),
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
        slk::Vector2f const spawn_margin = NEW_ENEMY_EXTENTS.max + SPAWN_MARGIN;
        slk::AABB2f env_spawn_space{slk::Vector2f(env_aabb.min + slk::Vector2f(0, env_aabb.height() * 0.444f)),
                                    env_aabb.max - slk::Vector2f(env_aabb.width() * .5f, 0)};
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

int main() {
    const WindowCtx WND_CTX = {.width = 1200, .height = 720};
    const slk::f32 CANNON_ROT_SPEED = 0.005f;

    InitWindow(WND_CTX.width, WND_CTX.height, "Sandbox");
    SetTraceLogLevel(LOG_TRACE);
    SetTargetFPS(60);

    GameState game_state;
    game_state.reset();

    std::vector<Ennemy> new_ennemies;
    new_ennemies.reserve(20);

    slk::f32 delta_time_ms = 0.f;
    auto start_time_point = std::chrono::high_resolution_clock::now();

    while (!WindowShouldClose()) {
        game_state.cannon.update(delta_time_ms);

        if (game_state.ennemies.empty() || IsKeyReleased(KEY_R)) {
            game_state.reset();
        }
        if (IsKeyDown(KEY_J) || IsKeyDown(KEY_LEFT)) {
            game_state.cannon.turn(CANNON_ROT_SPEED * delta_time_ms);
        }
        if (IsKeyDown(KEY_L) || IsKeyDown(KEY_RIGHT)) {
            game_state.cannon.turn(-CANNON_ROT_SPEED * delta_time_ms);
        }
        if (IsKeyReleased(KEY_SPACE)) {
            if (auto new_projectile = game_state.cannon.shoot()) {
                game_state.projectiles.push_back(*new_projectile);
            }
        }
        std::deque<int> my_d;
        my_d.push_front(1);
        // projectiles integration + environment collision
        for (auto proj_iter = begin(game_state.projectiles); proj_iter != end(game_state.projectiles);) {
            Projectile & proj = *proj_iter;

            proj.prev_pos = proj.curr_pos;
            proj.curr_pos = proj.prev_pos + proj.velocity * delta_time_ms;

            // out of environment bound
            if (!game_state.env_aabb.is_inside(proj.curr_pos)) {
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
            if (!game_state.env_aabb.is_inside(ennemy_world_aabb)) {
                slk::Vector2f normal = slk::Vector2f::ZERO;

                // simple snapping
#if 1
                const slk::Vector2f plane_normals[] = {
                    slk::Vector2f{-1.f, 0.f},
                    slk::Vector2f{0.f, -1.f},
                    slk::Vector2f{1.f, 0.f},
                    slk::Vector2f{0.f, 1.f},
                };

                auto diff_max = slk::max_components(ennemy_world_aabb.max - game_state.env_aabb.max, slk::Vector2f::zero());
                ennemy.position -= diff_max;

                for (slk::u32 i = 0; i != 2; ++i) {
                    normal += plane_normals[i] * (float)(diff_max.values[i] > 0);
                }

                auto diff_min = slk::min_components(ennemy_world_aabb.min - game_state.env_aabb.min, slk::Vector2f::zero());
                ennemy.position -= diff_min;

                for (slk::u32 i = 2; i != 4; ++i) {
                    normal += plane_normals[i] * (float)(diff_min.values[i & 1] < 0);
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

                        if (prev_world_ennemy1_aabb.min.x >= prev_world_ennemy2_aabb.max.x && world_ennemy1_aabb.min.x < world_ennemy2_aabb.max.x) {
                            ennemy_iter->position.x += (world_ennemy2_aabb.max.x - world_ennemy1_aabb.min.x) + std::numeric_limits<float>::epsilon();
                            normal += slk::Vector2f(1, 0);
                        }

                        if (prev_world_ennemy1_aabb.min.y >= prev_world_ennemy2_aabb.max.y && world_ennemy1_aabb.min.y < world_ennemy2_aabb.max.y) {
                            ennemy_iter->position.y += (world_ennemy2_aabb.max.y - world_ennemy1_aabb.min.y) + std::numeric_limits<float>::epsilon();
                            normal += slk::Vector2f(0, 1);
                        }

                        if (prev_world_ennemy1_aabb.max.x <= prev_world_ennemy2_aabb.min.x && world_ennemy1_aabb.max.x > world_ennemy2_aabb.min.x) {
                            ennemy_iter->position.x += (world_ennemy2_aabb.min.x - world_ennemy1_aabb.max.x) - std::numeric_limits<float>::epsilon();
                            normal += slk::Vector2f(-1, 0);
                        }
                        if (prev_world_ennemy1_aabb.max.y <= prev_world_ennemy2_aabb.min.y && world_ennemy1_aabb.max.y > world_ennemy2_aabb.min.y) {
                            ennemy_iter->position.y += (world_ennemy2_aabb.min.y - world_ennemy1_aabb.max.y) - std::numeric_limits<float>::epsilon();
                            normal += slk::Vector2f(0, -1);
                        }

                        // this could happen if the frame time is too long and the penetration too important
                        if (normal.x == 0 && normal.y == 0) {
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

            auto diff_max = slk::max_components(ennemy_aabb.max - game_state.env_aabb.max, slk::Vector2f::zero());
            ennemy.position -= diff_max;

            auto diff_min = slk::min_components(ennemy_aabb.min - game_state.env_aabb.min, slk::Vector2f::zero());
            ennemy.position -= diff_min;
        }

        // projectile/ennemy collisions
        for (auto proj_iter = begin(game_state.projectiles); proj_iter != end(game_state.projectiles);) {
            slk::b8 has_hit = false;
            for (auto ennemy_iter = begin(game_state.ennemies); ennemy_iter != end(game_state.ennemies); ++ennemy_iter) {
                slk::AABB2f world_aabb = ennemy_iter->extents.displaced(ennemy_iter->position);
                if (world_aabb.is_inside(proj_iter->curr_pos)) {
                    const slk::f32 curr_width = ennemy_iter->extents.width();
                    if (curr_width >= 32) {
                        const slk::AABB2f new_extents = ennemy_iter->extents * 0.5f;
                        const slk::Vector2f new_ennemy1 = ennemy_iter->position + new_extents.max;
                        const slk::Vector2f new_ennemy2 = ennemy_iter->position + new_extents.min;

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

        // Rendering
        BeginDrawing();

        ClearBackground(RAYWHITE);

        // Draw environment
        draw_aabb(WND_CTX, ENV_AABB, slk::Vector2f::zero(), slk::ColorU32{0, 0, 0, 255});

        // Draw canon
        draw_triangle(WND_CTX, game_state.cannon.getPos(), game_state.cannon.getRotation(), game_state.cannon.getWidth(),
                      game_state.cannon.getHeight(), slk::ColorU32{0, 0, 0, 255});

        // Compute ennemies' clock rotations
        auto const wall_clock_time = std::chrono::system_clock::now();
        auto const time = std::chrono::system_clock::to_time_t(wall_clock_time);
        auto const actual_time = std::localtime(&time);

        const slk::f32 curr_hour_angle = -(actual_time->tm_hour / 24.f) * 2 * slk::PI_F32;
        const slk::f32 curr_min_angle = -(actual_time->tm_min / 60.f) * 2 * slk::PI_F32;
        const slk::f32 curr_sec_angle = -(actual_time->tm_sec / 60.f) * 2 * slk::PI_F32;

        const slk::Matrix2f hour_rot = slk::Matrix2f::make_rotation(curr_hour_angle);
        const slk::Matrix2f min_rot = slk::Matrix2f::make_rotation(curr_min_angle);
        const slk::Matrix2f sec_rot = slk::Matrix2f::make_rotation(curr_sec_angle);

        // Render ennemies
        for (auto const & ennemy : game_state.ennemies) {
            draw_aabb(WND_CTX, ennemy.extents, ennemy.position, slk::ColorU32{255, 0, 0, 255});

            DrawLineEx(to_rvec2(WND_CTX, ennemy.position),
                       to_rvec2(WND_CTX, ennemy.position + slk::Vector2f::unitY() * hour_rot * ennemy.extents.width() * 0.2f), 2, RED);
            DrawLineEx(to_rvec2(WND_CTX, ennemy.position),
                       to_rvec2(WND_CTX, ennemy.position + slk::Vector2f::unitY() * min_rot * ennemy.extents.width() * 0.4f), 2, GREEN);
            DrawLineEx(to_rvec2(WND_CTX, ennemy.position),
                       to_rvec2(WND_CTX, ennemy.position + slk::Vector2f::unitY() * sec_rot * ennemy.extents.width() * 0.3f), 2, BLUE);
        }

        // Render projectiles
        for (auto & proj : game_state.projectiles) {
            const Vector2 vert1 = {.x = proj.prev_pos.x, .y = WND_CTX.height - proj.prev_pos.y};
            const Vector2 vert2 = {.x = proj.curr_pos.x, .y = WND_CTX.height - proj.curr_pos.y};

            DrawLineEx(vert1, vert2, 2, BLACK);
        }

        EndDrawing();

        const auto end_time_point = std::chrono::high_resolution_clock::now();
        const auto delta_time_point = end_time_point - start_time_point;

        delta_time_ms = static_cast<slk::f32>(std::chrono::duration_cast<std::chrono::microseconds>(delta_time_point).count() / 1000.0);
        start_time_point = end_time_point;
    }

    CloseWindow();

    return 0;
}
