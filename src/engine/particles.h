#ifndef PARTICLES_H
#define PARTICLES_H

#include <SDL3/SDL.h>
#include <vector>
#include <string>
#include <array>
#include <random>

struct Particle {
    SDL_FPoint pos, vel;
    SDL_FPoint accel = {0, 0};
    float life = 1.0f, size = 2.0f, rotation = 0.0f;  // Rotation for snow
    std::array<float, 4> color = {0.4f, 0.6f, 1.0f, 0.5f};  // RGBA
    float length = 15.0f;  // For rain streaks
};

class ParticleEmitter {
public:
    void spawn(const std::string& type, SDL_FPoint pos, int count);
    virtual void update(float dt) {}
    virtual void render(SDL_Renderer* renderer) {}
protected:
    std::vector<Particle> particles;
    SDL_FPoint emitter_pos;
    std::string type;
};

class FireEmitter : public ParticleEmitter {
public:
    FireEmitter(SDL_FPoint pos, int intensity = 1) {
        emitter_pos = pos;
        type = "fire";
        spawn_rate = 20 * intensity;
    }
    void update(float dt) override;
    void render(SDL_Renderer* renderer) override;
private:
    int spawn_rate = 20;
    float spawn_timer = 0.0f;
    void spawn_particle();
};

class SmokeEmitter : public ParticleEmitter {
public:
    SmokeEmitter(SDL_FPoint pos, int intensity = 1) {
        emitter_pos = pos;
        type = "smoke";
        spawn_rate = 15 * intensity;
    }
    void update(float dt, float wind_strength = 0.0f);
    void render(SDL_Renderer* renderer) override;
private:
    int spawn_rate = 15;
    float spawn_timer = 0.0f;
    void spawn_particle();
};

class RainEmitter : public ParticleEmitter {
public:
    RainEmitter(int screen_w, int screen_h, float intensity = 1.0f) {
        type = "rain";
        spawn_rate = 200 * intensity;
        width = screen_w;
        height = screen_h;
    }
    void update(float dt, float wind = 0.0f, float intensity = 1.0f);
    void render(SDL_Renderer* renderer) override;
private:
    int spawn_rate = 200;
    float spawn_timer = 0.0f;
    int width, height;
    float wind_speed = 0.0f;
    void spawn_drop();
};

class SnowEmitter : public ParticleEmitter {
public:
    SnowEmitter(int screen_w, int screen_h, float intensity = 1.0f) {
        type = "snow";
        spawn_rate = 100 * intensity;
        width = screen_w;
        height = screen_h;
    }
    void update(float dt, float wind = 0.0f);
    void render(SDL_Renderer* renderer) override;
private:
    int spawn_rate = 100;
    float spawn_timer = 0.0f;
    int width, height;
    float wind_speed = 0.0f;
    void spawn_flake();
};

class SplashEmitter : public ParticleEmitter {
public:
    SplashEmitter(SDL_FPoint pos) {
        emitter_pos = pos;
        type = "splash";
    }
    void update(float dt) override;
    void render(SDL_Renderer* renderer) override;
    void spawn_splash(int count = 5);
private:
    float spawn_timer = 0.0f;
};

class SparkEmitter : public ParticleEmitter {
public:
    SparkEmitter(SDL_FPoint pos, int count) {
        emitter_pos = pos;
        type = "spark";
        spawn(count);
    }
    void update(float dt) override;
    void render(SDL_Renderer* renderer) override;
private:
    void spawn(int count);
};

class FogEmitter : public ParticleEmitter {
public:
    FogEmitter(SDL_FPoint pos, int intensity = 1) {
        emitter_pos = pos;
        type = "fog";
        spawn_rate = 5 * intensity;
    }
    void update(float dt) override;
    void render(SDL_Renderer* renderer) override;
private:
    int spawn_rate = 5;
    float spawn_timer = 0.0f;
    void spawn_fog_blob();
};

class GrassSwayEmitter : public ParticleEmitter {
public:
    GrassSwayEmitter(SDL_FPoint pos, int blade_count = 10) {
        emitter_pos = pos;
        type = "grass_sway";
        spawn_blades(blade_count);
    }
    void update(float dt, float wind = 0.0f);
    void render(SDL_Renderer* renderer) override;
private:
    void spawn_blades(int count);
};

#endif
