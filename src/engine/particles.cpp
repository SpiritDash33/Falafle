#include "particles.h"
#include <random>
#include <algorithm>
#include <cmath>

std::random_device rd;
std::mt19937 gen(rd());
std::uniform_real_distribution<> dis(0.0f, 1.0f);  // Global for convenience

void ParticleEmitter::spawn(const std::string& type, SDL_FPoint pos, int count) {
    this->type = type;
    emitter_pos = pos;
    for (int i = 0; i < count; ++i) {
        Particle p;
        p.pos = pos;
        p.vel.x = dis(gen) * 40 - 20;
        p.vel.y = -dis(gen) * 50;
        p.life = dis(gen) * 1.5f + 0.5f;
        p.size = dis(gen) * 4 + 4;
        particles.push_back(p);
    }
}

// Base class implementations removed - they're defined in the header

// FireEmitter
void FireEmitter::update(float dt) {
    spawn_timer += dt;
    if (spawn_timer >= 1.0f / spawn_rate) {
        spawn_particle();
        spawn_timer = 0.0f;
    }
    ParticleEmitter::update(dt);
}

void FireEmitter::render(SDL_Renderer* renderer) {
    for (auto& p : particles) {
        float t = 1.0f - p.life / (p.life + 1.0f);
        p.color[0] = 255;
        p.color[1] = t * 165;
        p.color[2] = 0;
        p.color[3] = p.life * 0.8f + 0.2f;
    }
    ParticleEmitter::render(renderer);
}

void FireEmitter::spawn_particle() {
    Particle p;
    p.pos = emitter_pos;
    p.vel.x = dis(gen) * 40 - 20;
    p.vel.y = -dis(gen) * 80;
    p.life = dis(gen) * 0.8f + 0.2f;
    p.size = dis(gen) * 3 + 2;
    p.color = {1.0f, 0.0f, 0.0f, 1.0f};
    particles.push_back(p);
}

// SmokeEmitter
void SmokeEmitter::update(float dt, float wind_strength) {
    spawn_timer += dt;
    if (spawn_timer >= 1.0f / spawn_rate) {
        spawn_particle();
        spawn_timer = 0.0f;
    }

    for (auto& p : particles) {
        p.pos.x += p.vel.x * dt + wind_strength * 20 * dt;
        p.pos.y += p.vel.y * dt;
        p.vel.y += p.accel.y * dt;
        p.life -= dt * 0.5f;
        p.size += dt * 2;
        p.color[3] = p.life * 0.8f;
    }
    particles.erase(std::remove_if(particles.begin(), particles.end(), [](const Particle& p){ return p.life <= 0; }), particles.end());
}

void SmokeEmitter::render(SDL_Renderer* renderer) {
    for (auto& p : particles) {
        float t = 1.0f - p.life;
        p.color[0] = 100 + t * 100;
        p.color[1] = 100 + t * 100;
        p.color[2] = 100 + t * 155;
        SDL_SetRenderDrawColor(renderer, p.color[0], p.color[1], p.color[2], p.color[3]*255);
        SDL_FRect quad = {p.pos.x - p.size/2, p.pos.y - p.size/2, p.size, p.size};
        SDL_RenderFillRect(renderer, &quad);
    }
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
}

void SmokeEmitter::spawn_particle() {
    Particle p;
    p.pos = emitter_pos;
    p.vel.x = dis(gen) * 100 - 50;
    p.vel.y = -dis(gen) * 30;
    p.life = dis(gen) * 3 + 2;
    p.size = dis(gen) * 4 + 8;
    p.color = {0.4f, 0.4f, 0.5f, 0.5f};
    particles.push_back(p);
}

// RainEmitter
void RainEmitter::update(float dt, float wind, float intensity) {
    wind_speed = wind;
    spawn_rate = 200 * intensity;
    spawn_timer += dt;
    if (spawn_timer >= 1.0f / spawn_rate) {
        spawn_drop();
        spawn_timer = 0.0f;
    }

    for (auto& p : particles) {
        p.pos.x += (p.vel.x + wind_speed) * dt;
        p.pos.y += p.vel.y * dt;
        if (p.pos.y > height + p.length) {
            p.pos.x = dis(gen) * width;
            p.pos.y = -p.length;
        }
        if (p.pos.x < -10 || p.pos.x > width + 10) p.pos.x = dis(gen) * width;
    }
    while (particles.size() > 500) particles.erase(particles.begin());
}

void RainEmitter::render(SDL_Renderer* renderer) {
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 100, 150, 255, 128);
    for (const auto& p : particles) {
        SDL_FPoint end = {p.pos.x - p.vel.x * (p.length / 300.0f), p.pos.y - p.vel.y * (p.length / 300.0f)};
        SDL_RenderLine(renderer, p.pos.x, p.pos.y, end.x, end.y);
    }
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
}

void RainEmitter::spawn_drop() {
    Particle p;
    p.pos.x = dis(gen) * width;
    p.pos.y = -dis(gen) * 20;
    p.vel.x = dis(gen) * 20 - 10;
    p.vel.y = 300 + dis(gen) * 100;
    p.life = 1.0f;
    p.length = dis(gen) * 10 + 10;
    p.size = 1.5f + dis(gen) * 1.0f;
    particles.push_back(p);
}

// SnowEmitter
void SnowEmitter::update(float dt, float wind) {
    wind_speed = wind;
    spawn_timer += dt;
    if (spawn_timer >= 1.0f / spawn_rate) {
        spawn_flake();
        spawn_timer = 0.0f;
    }

    for (auto& p : particles) {
        p.pos.x += (p.vel.x + wind_speed * 5) * dt;
        p.pos.y += p.vel.y * dt;
        p.rotation += dt * 2;
        if (p.pos.y > height) {
            p.pos.y = -p.size * 2;
            p.pos.x = dis(gen) * width;
        }
    }
}

void SnowEmitter::render(SDL_Renderer* renderer) {
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 200);
    for (const auto& p : particles) {
        SDL_FRect quad = {p.pos.x - p.size/2, p.pos.y - p.size/2, p.size, p.size};
        SDL_RenderFillRect(renderer, &quad);
    }
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
}

void SnowEmitter::spawn_flake() {
    Particle p;
    p.pos.x = dis(gen) * width;
    p.pos.y = -dis(gen) * 10;
    p.vel.x = dis(gen) * 20 - 10;
    p.vel.y = 80 + dis(gen) * 70;
    p.life = 1.0f;
    p.size = dis(gen) * 4 + 2;
    p.rotation = dis(gen) * 2 * M_PI;
    particles.push_back(p);
}

// SplashEmitter
void SplashEmitter::update(float dt) {
    for (auto& p : particles) {
        p.pos.x += p.vel.x * dt;
        p.pos.y += p.vel.y * dt;
        p.vel.y += 200 * dt;
        p.life -= dt;
        p.color[3] *= 0.95f;
    }
    particles.erase(std::remove_if(particles.begin(), particles.end(), [](const Particle& p){ return p.life <= 0; }), particles.end());
}

void SplashEmitter::render(SDL_Renderer* renderer) {
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 100, 150, 255, 128);
    for (const auto& p : particles) {
        SDL_FRect quad = {p.pos.x - p.size/2, p.pos.y - p.size/2, p.size, p.size};
        SDL_RenderFillRect(renderer, &quad);
    }
}

void SplashEmitter::spawn_splash(int count) {
    for (int i = 0; i < count; ++i) {
        Particle p;
        p.pos = emitter_pos;
        p.vel.x = dis(gen) * 100 - 50;
        p.vel.y = -dis(gen) * 50;
        p.life = dis(gen) * 0.3 + 0.2;
        p.size = dis(gen) * 2 + 1;
        p.color = {0.4f, 0.6f, 1.0f, 0.7f};
        particles.push_back(p);
    }
}

// SparkEmitter
void SparkEmitter::update(float dt) {
    for (auto& p : particles) {
        p.pos.x += p.vel.x * dt;
        p.pos.y += p.vel.y * dt;
        p.vel.x *= 0.95f;
        p.vel.y *= 0.95f;
        p.life -= dt * 2;
        p.color[3] *= 0.9f;
        p.size *= 0.98f;
    }
    particles.erase(std::remove_if(particles.begin(), particles.end(), [](const Particle& p){ return p.life <= 0; }), particles.end());
}

void SparkEmitter::render(SDL_Renderer* renderer) {
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_ADD);
    SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
    for (const auto& p : particles) {
        SDL_FRect quad = {p.pos.x - p.size/2, p.pos.y - p.size/2, p.size, p.size};
        SDL_RenderFillRect(renderer, &quad);
    }
}

void SparkEmitter::spawn(int count) {
    for (int i = 0; i < count; ++i) {
        Particle p;
        p.pos = emitter_pos;
        p.vel.x = dis(gen) * 200 - 100;
        p.vel.y = dis(gen) * 200 - 100;
        p.life = dis(gen) * 0.5 + 0.5;
        p.size = dis(gen) * 3 + 1;
        p.color = {1.0f, 1.0f, 0.0f, 1.0f};
        particles.push_back(p);
    }
}

// FogEmitter
void FogEmitter::update(float dt) {
    spawn_timer += dt;
    if (spawn_timer >= 1.0f / spawn_rate) {
        spawn_fog_blob();
        spawn_timer = 0.0f;
    }

    for (auto& p : particles) {
        p.pos.x += p.vel.x * dt * 0.5f;
        p.pos.y += p.vel.y * dt;
        p.life -= dt * 0.2f;
        p.size += dt * 0.5f;
        p.color[3] = p.life * 0.3f + 0.1f;
    }
    particles.erase(std::remove_if(particles.begin(), particles.end(), [](const Particle& p){ return p.life <= 0; }), particles.end());
}

void FogEmitter::render(SDL_Renderer* renderer) {
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_ADD);
    SDL_SetRenderDrawColor(renderer, 150, 150, 150, 50);
    for (const auto& p : particles) {
        SDL_FRect quad = {p.pos.x - p.size/2, p.pos.y - p.size/2, p.size, p.size};
        SDL_RenderFillRect(renderer, &quad);
    }
}

void FogEmitter::spawn_fog_blob() {
    Particle p;
    p.pos = emitter_pos;
    p.vel.x = dis(gen) * 20 - 10;
    p.vel.y = dis(gen) * 10;
    p.life = dis(gen) * 5 + 3;
    p.size = dis(gen) * 10 + 5;
    p.color = {0.6f, 0.6f, 0.6f, 0.3f};
    particles.push_back(p);
}

// GrassSwayEmitter
void GrassSwayEmitter::update(float dt, float wind) {
    for (auto& p : particles) {
        p.rotation += wind * 5 * dt;
        p.pos.y += std::sin(p.rotation) * dt * 2;
    }
}

void GrassSwayEmitter::render(SDL_Renderer* renderer) {
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0, 100, 0, 255);
    for (const auto& p : particles) {
        SDL_FPoint tip = {p.pos.x + std::sin(p.rotation) * 10, p.pos.y - 15};
        SDL_RenderLine(renderer, p.pos.x, p.pos.y, tip.x, tip.y);
    }
}

void GrassSwayEmitter::spawn_blades(int count) {
    for (int i = 0; i < count; ++i) {
        Particle p;
        p.pos.x = emitter_pos.x + dis(gen) * 64 - 32;
        p.pos.y = emitter_pos.y;
        p.vel = {0, 0};
        p.life = 1.0f;
        p.size = 1.0f;
        p.rotation = dis(gen) * 2 * M_PI;
        particles.push_back(p);
    }
}
