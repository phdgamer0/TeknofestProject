#pragma once
#include "comms.hpp"
#include "constants.hpp"
#include "logger.hpp"
#include "logic.hpp"
#include "physichs.hpp"
#include <cstdint>
#include <unordered_map>
#include <unordered_set>

typedef struct ImageFrame {
   vec centerPos;                                        // position of the center of the camera frame in (x,y,z) coordinates
   double frame_width = 1920.0F, frame_height = 1080.0F; // dimensions of the camera frame in pixels
   uint64_t timestamp{0};                                // the time where the frame was captured
   std::unordered_map<uint8_t, Object> detectedObjects;  // list of detected objects in the current frame
   ImageFrame(vec centerPos = vec(0, 0, 0), uint64_t timestamp = 0) : centerPos(centerPos), timestamp(timestamp) {
      detectedObjects.reserve(16);
   }
   // Prevent expensive copying: frames must be passed by pointer/reference or updated in-place
   ImageFrame(const ImageFrame&) = delete;
   ImageFrame& operator=(const ImageFrame&) = delete;
   ImageFrame(ImageFrame&&) = default;
   ImageFrame& operator=(ImageFrame&&) = default;
   friend std::ostream& operator<<(std::ostream& os, const ImageFrame& frame);
   ~ImageFrame() = default;
} img;

class Camera {
 public:
   vec CamCenterPos;                   // Camera position
   double frame_width, frame_height;   // dimensions of the camera frame in pixels
   void calibrate(vec newPos);         // calibrate the camera to a new position (smooth transition)
   void reset();                       // reset camera to default position and parameters
   void updateCamera(float deltaTime); // update camera position and parameters based on velocity and acceleration (called every frame)
   constexpr Camera(vec pos = vec(0, 0, 0), double width = 1920.0F, double height = 1080.0F) : CamCenterPos(pos), frame_width(width), frame_height(height) {};
   friend std::ostream& operator<<(std::ostream& os, const Camera& cam);
   ~Camera() = default;
};

class Laser {
 public:
   bool isOn; // Laser state (on/off)
   vec laserPos;
   constexpr Laser(bool state = false) : isOn(state) {};
   void turnOn();  // turn the laser on
   void turnOff(); // turn the laser off
   friend std::ostream& operator<<(std::ostream& os, const Laser& laser);
   ~Laser() = default;
};

class Engine {
 public:
   vec velocity;     // Engine velocity
   vec acceleration; // Engine acceleration
   bool isActive;    // Engine state (active/inactive)
   constexpr Engine(vec vel = vec(0, 0, 0), vec acc = vec(0, 0, 0), bool active = false) : velocity(vel), acceleration(acc), isActive(active) {};
   void move(vec vel, vec acc = vec(0, 0, 0));      // move the engine with specified velocity and optional acceleration
   void stop();                                     // stop the engine (set velocity and acceleration to zero)
   void calibrate(vec vel, vec acc = vec(0, 0, 0)); // calibrate the engine to a new velocity and acceleration (smooth transition)
   void reset();                                    // reset the engine to default parameters (velocity and acceleration to zero)
   void updateEngine(float deltaTime);              // update engine velocity and acceleration based on current state (called every frame)
   friend std::ostream& operator<<(std::ostream& os, const Engine& engine);
   ~Engine() = default;
};

class System {
 public:
   CommunicatorDevice COMM_CHANNEL; // System communication channel
   Camera& camera;                  // Camera instance
   Engine& engineLeftYaw;           // Left yaw Engine instance
   Engine& engineRightYaw;          // Right yaw Engine instance
   Engine& enginePitch;             // Pitch Engine instance
   Laser& laser;                    // Laser instance
   Brain& brain;                    // Brain instance owned by System
   Logger& logger;                  // Logger instance owned by System
   double yawAngle{0.0};            // Yaw angle of the system in degrees
   double pitchAngle{0.0};          // Pitch angle of the system in degrees
   bool isShutDown{false};          // System state (shut down or running)
   void executeSignal(sig signal);  // execute a received signal and perform the corresponding action
   void shutdown();                 // shut down the system
   void startup();                  // startup the system
   void reset();                    // reset the system
   void tick(float deltaTime);      // synchronous per-frame update for camera, engines, and kinematics
   System(Camera& cam, Engine& engineLeftYaw, Engine& engineRightYaw, Engine& enginePitch, Laser& laser, Brain& brain, Logger& logger);
   ~System() = default;
   // Circular ring buffer frame management: in-place overwrite, zero heap allocation
   ImageFrame* pushFrame(vec centerPos = vec(0, 0, 0), uint64_t timestamp = 0);
   // Accessors for state
   std::unordered_map<uint8_t, Object>& getTrackedObjects();
   const std::unordered_map<uint8_t, Object>& getTrackedObjects() const;
   ImageFrame** getHistoryFrames();
   ImageFrame* const* getHistoryFrames() const;
   // Update kinematics using the internal Brain
   void updateKinematics();
   friend std::ostream& operator<<(std::ostream& os, const System& system);

 private:
   std::unordered_map<uint8_t, Object> trackedObjects;
   std::unordered_set<uint8_t> destroyedTargets;
   ImageFrame framePool[HISTORY_FRAME_COUNTS];
   ImageFrame* HistoryFrames[HISTORY_FRAME_COUNTS]{};
   size_t frameCount{0};
};