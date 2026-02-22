#pragma once
#include "physichs.hpp"
#include "comms.hpp"
#include "logic.hpp"
#include <vector>
#include <unordered_map>
extern const double MAX_VELOCUTY_X;
extern const double MAX_VELOCUTY_Y;
extern const vec CAMERA_LASER_OFFSET; // offset of the laser from the camera center in the (x,y,z) directions
extern std::unordered_map<uint8_t, Object> trackedObjects; // map to keep track of detected objects by their ID
typedef struct ImageFrame{
    vec centerPos; // position of the center of the camera frame in (x,y,z) coordinates
    double frame_width=1920.0F, frame_height=1080.0F; // dimensions of the camera frame in pixels
    std::vector<obj> detectedObjects; // list of detected objects in the current frame
    ImageFrame(vec centerPos = vec(0,0,0)) : centerPos(centerPos) {};
    friend std::ostream& operator<<(std::ostream& os, const ImageFrame& frame);
    ~ImageFrame() = default;
}img;
class AI{
public:
    void operator<<(const img& frame); // process an image frame and determine the appropriate response (called every frame for each image frame)
private:
    void processImage(const img& frame); // process an image frame and determine the appropriate response (called every frame for each image frame)
    ~AI() = default;
};
class Camera {
public:
    vec CamCenterPos; // Camera position
    double frame_width, frame_height; // dimensions of the camera frame in pixels
    Camera(vec pos = vec(0,0,0), double width = 1920.0F, double height = 1080.0F) : CamCenterPos(pos), frame_width(width), frame_height(height) {};
    [[maybe_unused]]void calibrate(vec newPos); // calibrate the camera to a new position (smooth transition)
    void reset(); // reset camera to default position and parameters
    void updateCamera(float deltaTime); // update camera position and parameters based on velocity and acceleration (called every frame)
    img captureFrame();
    friend std::ostream& operator<<(std::ostream& os, const Camera& cam);
    ~Camera() = default;
};
class Laser{
public:
    bool isOn; // Laser state (on/off)
    vec laserPos;
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
    void handleSystemSignal(sig signal); // handle system-related signals
    void handleObjectSignal(sig signal); // handle object-related signals
};