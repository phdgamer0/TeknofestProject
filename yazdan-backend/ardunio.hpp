#pragma once
#include "physichs.hpp"
#include "comms.hpp"
#include "logic.hpp"
extern const double MAX_VELOCUTY_X;
extern const double MAX_VELOCUTY_Y;
class Camera {
public:
    vec CamCenterPos; // Camera position
    vec CamCenterVel; // Camera velocity
    vec CamCenterAcc; // Camera acceleration
    double CamFOV; // Camera field of view in degrees
    double CamAspectRatio; // Camera aspect ratio (width/height)
    double CamNearPlane; // Near clipping plane distance
    double CamFarPlane; // Far clipping plane distance
    Camera(vec pos = vec(0,0,0), vec vel = vec(0,0,0), vec acc = vec(0,0,0), 
    double fov = 90.0, double aspectRatio = 16.0/9.0, double nearPlane = 0.5f, double farPlane = 20.0f) :
    CamCenterPos(pos), CamCenterVel(vel), CamCenterAcc(acc), CamFOV(fov), 
    CamAspectRatio(aspectRatio), CamNearPlane(nearPlane), CamFarPlane(farPlane) {};
    ~Camera() = default;
    void move(vec pos, vec vel = vec(0,0,0), vec acc = vec(0,0,0)); // move camera to a new position with optional velocity and acceleration
    void calibrate(vec pos, vec vel = vec(0,0,0), vec acc = vec(0,0,0)); // calibrate camera to a new position with optional velocity and acceleration (smooth transition)
    void reset(); // reset camera to default position and parameters
    void updateCamera(float deltaTime); // update camera position and parameters based on velocity and acceleration (called every frame)
    friend std::ostream& operator<<(std::ostream& os, const Camera& cam);
    ~Camera() = default;
};
class Laser{
public:
    bool isOn; // Laser state (on/off)
    Laser(bool state = false) : isOn(state) {};
    void turnOn(); // turn the laser on
    void turnOff(); // turn the laser off
    friend std::ostream& operator<<(std::ostream& os, const Laser& laser);
    ~Laser() = default;
};
class Engine{
public:
    vec velocity; // Engine velocity
    vec acceleration; // Engine acceleration
    bool isActive; // Engine state (active/inactive)
    Engine(vec vel = vec(0,0,0), vec acc = vec(0,0,0), bool active = false) : velocity(vel), acceleration(acc), isActive(active) {};
    void move(vec vel, vec acc = vec(0,0,0)); // move the engine with specified velocity and optional acceleration
    void stop(); // stop the engine (set velocity and acceleration to zero)
    void calibrate(vec vel, vec acc = vec(0,0,0)); // calibrate the engine to a new velocity and acceleration (smooth transition)
    void reset(); // reset the engine to default parameters (velocity and acceleration to zero)
    void updateEngine(float deltaTime); // update engine velocity and acceleration based on current state (called every frame)
    friend std::ostream& operator<<(std::ostream& os, const Engine& engine);
    ~Engine() = default;
};
class System{
public:
    void executeSignal(sig signal); // execute a received signal and perform the corresponding action
    double yawAngle; // Yaw angle of the system in degrees
    double pitchAngle; // Pitch angle of the system in degrees
    bool isShutDown; // System state (shut down or running)
    System(bool shutDown = false) : isShutDown(shutDown) {};
    void shutdown(); // shut down the system
    void restart(); // restart the system
    friend std::ostream& operator<<(std::ostream& os, const System& system);
    ~System() = default;
private:
    void handleCameraSignal(sig signal); // handle camera-related signals
    void handleEngineSignal(sig signal); // handle engine-related signals
    void handleLaserSignal(sig signal); // handle laser-related signals
};