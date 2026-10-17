#define Camera RaylibCamera
#include <raylib.h>
#undef Camera

#include "../include/ardunio.hpp"
#include "../include/constants.hpp"
#include "../include/logger.hpp"
#include "../include/logic.hpp"
#include "../include/physichs.hpp"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

// Convert Teknofest coordinates (x: forward, y: right, z: up) to Raylib 3D world (X: right, Y: up, Z: forward)
inline Vector3 toRaylib(const vec& v) {
   return Vector3{static_cast<float>(v.y), static_cast<float>(v.z), static_cast<float>(v.x)};
}

// Calculate shortest distance from a 3D point P to a 3D line segment AB
inline double DistancePointToSegment(const vec& p, const vec& a, const vec& b) {
   vec ab = b - a;
   double abLenSq = ab * ab;
   if (abLenSq < 1e-12) {
      return p.distanceTo(a);
   }
   vec ap = p - a;
   double t = (ap * ab) / abLenSq;
   t = std::clamp(t, 0.0, 1.0);
   vec closest = a + (ab * t);
   return p.distanceTo(closest);
}

// Active visualizer scenario mode
enum class ScenarioMode {
   STATIONARY_TARGET = 1,
   CROSSING_FAST_TARGET = 2,
   RADIAL_DIVE_BOMBER = 3,
   ACCELERATING_TARGET = 4,
   ZIG_ZAG_MANEUVER = 5,
   FRIENDLY_IN_LINE_OF_FIRE = 6,
   MULTI_TARGET_PRIORITY = 7,
   RANDOM_SWARM = 8
};

// Simulated physical balloon
struct SimBalloon {
   uint8_t id = 0;
   vec pos{0, 0, 0};
   vec vel{0, 0, 0};
   vec acc{0, 0, 0};
   bool isFriendly = false;
   bool isAlive = true;
   double health = 1.0;
   Color color = RED;
};

// 3D Explosion Particle
struct Particle {
   Vector3 pos;
   Vector3 vel;
   Color color;
   float alpha;
   float life;
};

struct VisualizerState {
   ScenarioMode mode = ScenarioMode::STATIONARY_TARGET;
   bool scopeView = false; // false = 3D Orbit Cam, true = Turret Scope Cam
   bool paused = false;
   int shaderMode = 0; // 0 = Tactical HUD, 1 = FLIR Thermal, 2 = Night Vision, 3 = Clean
   float simTime = 0.0f;
   uint64_t frameCounter = 0;
   uint8_t nextId = 10;
   int targetsNeutralized = 0;
   int friendlyCasualties = 0;
   float announcementTimer = 0.0f;
   std::string announcementText = "";
   std::vector<SimBalloon> simBalloons;
   std::vector<Particle> particles;
   bool friendlyObstructed = false;
   float friendlyHitWarning = 0.0f;
};

// GLSL Fragment Shader for GPU-accelerated Tactical Scope and Post-Processing
const char* g_tacticalShaderFS = R"(#version 330 core
in vec2 fragTexCoord;
in vec4 fragColor;
out vec4 finalColor;

uniform sampler2D texture0;
uniform vec4 colDiffuse;
uniform float uTime;
uniform vec2 uResolution;
uniform int uShaderMode;      // 0 = Tactical HUD, 1 = FLIR Thermal, 2 = Night Vision, 3 = Clean
uniform float uLaserIntensity; // > 0.0 when laser is active

void main() {
    vec2 uv = fragTexCoord;
    vec4 baseColor = texture(texture0, uv);

    if (uShaderMode == 3) {
        finalColor = baseColor;
        return;
    }

    vec2 center = uv - vec2(0.5, 0.5);
    float dist = length(center);
    float vignette = smoothstep(0.88, 0.30, dist);

    if (uShaderMode == 0) {
        // Mode 0: Tactical Combat Optic HUD
        float ca = 0.0016 + (uLaserIntensity * 0.003);
        float r = texture(texture0, uv + vec2(ca, 0.0)).r;
        float g = baseColor.g;
        float b = texture(texture0, uv - vec2(ca, 0.0)).b;
        vec3 col = vec3(r, g, b) * vignette;

        // Subtle CRT / scope scanline
        float scanline = sin(uv.y * uResolution.y * 1.2) * 0.03;
        col -= scanline;

        // Tactical optic tint
        col = mix(col, vec3(col.r * 0.9, col.g * 1.05, col.b * 1.15), 0.16);

        // Laser beam energy pulse
        if (uLaserIntensity > 0.0) {
            float pulse = sin(uTime * 30.0) * 0.06 + 0.94;
            col.r += uLaserIntensity * 0.18 * pulse;
        }

        finalColor = vec4(col, baseColor.a);
    }
    else if (uShaderMode == 1) {
        // Mode 1: FLIR / Thermal Infrared Optic
        float lum = dot(baseColor.rgb, vec3(0.299, 0.587, 0.114));
        float isHot = clamp((baseColor.r - baseColor.g * 0.8 - baseColor.b * 0.8) * 2.2, 0.0, 1.0);
        float isCool = clamp((baseColor.b - baseColor.r * 0.5) * 1.8, 0.0, 1.0);

        vec3 coldCol  = vec3(0.04, 0.08, 0.22);
        vec3 midCol   = vec3(0.68, 0.14, 0.58);
        vec3 hotCol   = vec3(1.0, 0.65, 0.1);
        vec3 ultraHot = vec3(1.0, 1.0, 0.95);

        vec3 thermal;
        if (lum < 0.25) {
            thermal = mix(coldCol, midCol, lum / 0.25);
        } else if (lum < 0.65) {
            thermal = mix(midCol, hotCol, (lum - 0.25) / 0.40);
        } else {
            thermal = mix(hotCol, ultraHot, (lum - 0.65) / 0.35);
        }

        // Boost active heat signatures
        thermal = mix(thermal, ultraHot, isHot * 0.90);
        thermal = mix(thermal, vec3(0.2, 0.8, 1.0), isCool * 0.75);

        // Sensor noise
        float noise = fract(sin(dot(uv * uTime, vec2(12.9898, 78.233))) * 43758.5453) * 0.035;
        thermal += noise - (sin(uv.y * uResolution.y * 1.0) * 0.03);

        finalColor = vec4(thermal * vignette, baseColor.a);
    }
    else if (uShaderMode == 2) {
        // Mode 2: Night Vision (NVG Green Phosphor Tube)
        float lum = dot(baseColor.rgb, vec3(0.299, 0.587, 0.114));
        float noise = fract(sin(dot(uv * uTime, vec2(23.14, 45.67))) * 43758.5453) * 0.05;
        vec3 nvg = vec3(0.12, 0.95, 0.28) * (lum * 1.5 + noise);
        finalColor = vec4(nvg * vignette, baseColor.a);
    }
}
)";

// Spawn explosion burst particles when a balloon is popped
void SpawnExplosion(VisualizerState& state, Vector3 pos, Color col) {
   for (int i = 0; i < 35; ++i) {
      Particle p;
      p.pos = pos;
      float vx = ((rand() % 200) - 100) / 25.0f;
      float vy = ((rand() % 200) - 30) / 20.0f;
      float vz = ((rand() % 200) - 100) / 25.0f;
      p.vel = Vector3{vx, vy, vz};
      p.color = (i % 2 == 0) ? col : YELLOW;
      p.life = 0.8f + (rand() % 40) / 100.0f;
      p.alpha = 1.0f;
      state.particles.push_back(p);
   }
}

// Draw a stylized 3D balloon (Charred Gray when destroyed)
void DrawBalloon(Vector3 pos, Color color, bool isTracked, double health, bool isAlive) {
   Color balloonCol = isAlive ? color : Color{95, 100, 110, 255}; // Charred metallic gray when destroyed

   // Main balloon body (sphere)
   DrawSphere(pos, 0.48f, balloonCol);
   DrawSphereWires(pos, 0.49f, 8, 8, ColorAlpha(isAlive ? DARKGRAY : BLACK, 0.4f));

   // Knot underneath
   Vector3 knotPos = {pos.x, pos.y - 0.48f, pos.z};
   DrawCylinder(knotPos, 0.09f, 0.03f, 0.10f, 6, balloonCol);

   // Dangling string
   Vector3 stringEnd = {pos.x, pos.y - 1.2f, pos.z};
   DrawLine3D(knotPos, stringEnd, isAlive ? LIGHTGRAY : DARKGRAY);

   // Only active alive targets have target brackets and health bars
   if (isAlive && isTracked) {
      DrawCubeWires(pos, 1.35f, 1.7f, 1.35f, RED);
      DrawCubeWires(pos, 1.40f, 1.75f, 1.40f, ColorAlpha(YELLOW, 0.8f));

      // 3D Health bar above balloon
      Vector3 barStart = {pos.x - 0.5f, pos.y + 0.85f, pos.z};
      Vector3 barEnd = {pos.x + 0.5f, pos.y + 0.85f, pos.z};
      Vector3 barCur = {pos.x - 0.5f + 1.0f * static_cast<float>(std::clamp(health, 0.0, 1.0)), pos.y + 0.85f, pos.z};
      DrawLine3D(barStart, barEnd, DARKGRAY);
      DrawLine3D(barStart, barCur, health > 0.3 ? LIME : RED);
   }
}

// Draw a stylized 3D turret at origin (Only drawn in Orbit view so it never obstructs 1st-person scope)
void DrawTurret(double yawDeg, double pitchDeg, bool laserFiring, Vector3 aimPoint) {
   // Base platform
   DrawCylinder(Vector3{0, -0.3f, 0}, 1.6f, 1.6f, 0.6f, 20, DARKGRAY);
   DrawCylinderWires(Vector3{0, -0.3f, 0}, 1.61f, 1.61f, 0.6f, 20, BLACK);

   // Rotating azimuth ring
   DrawCylinder(Vector3{0, 0.05f, 0}, 1.2f, 1.2f, 0.2f, 16, GRAY);

   // Turret sphere / yoke
   Vector3 turretCenter = {0, 0.6f, 0};
   DrawSphere(turretCenter, 0.7f, DARKGRAY);
   DrawSphereWires(turretCenter, 0.71f, 12, 12, BLACK);

   // Gun barrel
   double yawRad = toRadians(yawDeg);
   double pitchRad = toRadians(pitchDeg);

   float dirX = static_cast<float>(std::sin(yawRad) * std::cos(pitchRad));
   float dirY = static_cast<float>(std::sin(pitchRad));
   float dirZ = static_cast<float>(std::cos(yawRad) * std::cos(pitchRad));
   Vector3 barrelEnd = {turretCenter.x + dirX * 1.8f, turretCenter.y + dirY * 1.8f, turretCenter.z + dirZ * 1.8f};

   DrawCylinderEx(turretCenter, barrelEnd, 0.14f, 0.10f, 12, BLACK);

   // Laser offset emitter position (accounting for CAMERA_LASER_OFFSET)
   vec laserOffsetTk = CAMERA_LASER_OFFSET;
   Vector3 laserEmitter = {turretCenter.x + static_cast<float>(laserOffsetTk.y),
                           turretCenter.y + static_cast<float>(laserOffsetTk.z),
                           turretCenter.z + static_cast<float>(laserOffsetTk.x)};

   DrawSphere(laserEmitter, 0.06f, RED);

   // Laser beam if weapon is active
   if (laserFiring) {
      DrawLine3D(laserEmitter, aimPoint, RED);
      DrawLine3D(Vector3{laserEmitter.x, laserEmitter.y + 0.02f, laserEmitter.z}, aimPoint, MAROON);
      DrawLine3D(Vector3{laserEmitter.x + 0.02f, laserEmitter.y, laserEmitter.z}, aimPoint, ORANGE);
      DrawLine3D(laserEmitter, aimPoint, WHITE);

      // Impact glowing hot spot
      DrawSphere(aimPoint, 0.20f, RED);
      DrawSphere(aimPoint, 0.12f, YELLOW);
      DrawSphere(aimPoint, 0.06f, WHITE);
   }
}

// Load and configure test scenarios
void LoadScenario(ScenarioMode mode, System& sys, VisualizerState& state) {
   state.mode = mode;
   state.simTime = 0.0f;
   state.frameCounter = 0;
   state.simBalloons.clear();
   state.particles.clear();
   state.friendlyObstructed = false;
   state.friendlyHitWarning = 0.0f;
   sys.reset();

   switch (mode) {
   case ScenarioMode::STATIONARY_TARGET: {
      state.announcementText = "SCENARIO 1: STATIONARY TARGET LOCK & ENGAGE";
      state.announcementTimer = 3.0f;
      SimBalloon b;
      b.id = 1;
      b.pos = vec(10.0, 0.0, 1.5);
      b.vel = vec(0, 0, 0);
      b.acc = vec(0, 0, 0);
      b.isFriendly = false;
      b.color = Color{244, 67, 54, 255};
      state.simBalloons.push_back(b);
      break;
   }
   case ScenarioMode::CROSSING_FAST_TARGET: {
      state.announcementText = "SCENARIO 2: FAST CROSSING TARGET LEAD TRACKING";
      state.announcementTimer = 3.0f;
      SimBalloon b;
      b.id = 2;
      b.pos = vec(12.0, -6.5, 2.0);
      b.vel = vec(0, 4.2, 0);
      b.acc = vec(0, 0, 0);
      b.isFriendly = false;
      b.color = Color{244, 67, 54, 255};
      state.simBalloons.push_back(b);
      break;
   }
   case ScenarioMode::RADIAL_DIVE_BOMBER: {
      state.announcementText = "SCENARIO 3: HIGH-SPEED RADIAL DIVE BOMBER INTERCEPTION";
      state.announcementTimer = 3.0f;
      SimBalloon b;
      b.id = 3;
      b.pos = vec(22.0, 0.0, 5.0);
      b.vel = vec(-7.5, 0, -1.4);
      b.acc = vec(0, 0, 0);
      b.isFriendly = false;
      b.color = Color{244, 67, 54, 255};
      state.simBalloons.push_back(b);
      break;
   }
   case ScenarioMode::ACCELERATING_TARGET: {
      state.announcementText = "SCENARIO 4: ACCELERATING TARGET TRAJECTORY PREDICTION";
      state.announcementTimer = 3.0f;
      SimBalloon b;
      b.id = 4;
      b.pos = vec(14.0, -5.0, 1.0);
      b.vel = vec(0, 0.6, 0.2);
      b.acc = vec(0, 1.4, 0.6);
      b.isFriendly = false;
      b.color = Color{244, 67, 54, 255};
      state.simBalloons.push_back(b);
      break;
   }
   case ScenarioMode::ZIG_ZAG_MANEUVER: {
      state.announcementText = "SCENARIO 5: EVASIVE ZIG-ZAG TARGET PURSUIT";
      state.announcementTimer = 3.0f;
      SimBalloon b;
      b.id = 5;
      b.pos = vec(11.0, -3.0, 2.0);
      b.vel = vec(0, 3.5, 0);
      b.acc = vec(0, 0, 0);
      b.isFriendly = false;
      b.color = Color{244, 67, 54, 255};
      state.simBalloons.push_back(b);
      break;
   }
   case ScenarioMode::FRIENDLY_IN_LINE_OF_FIRE: {
      state.announcementText = "SCENARIO 6: FRIENDLY SAFEGUARD (HOLD FIRE WHILE OBSTRUCTED)";
      state.announcementTimer = 3.0f;
      // Hostile target directly down range at X = 12m, Z = 1.0m
      SimBalloon en;
      en.id = 11;
      en.pos = vec(12.0, 0.0, 1.0);
      en.vel = vec(0, 0, 0);
      en.acc = vec(0, 0, 0);
      en.isFriendly = false;
      en.color = Color{244, 67, 54, 255};
      state.simBalloons.push_back(en);

      // Friendly positioned at X = 6.0m, Z = 1.0m (matching line of fire height!)
      // Oscillates across Y = -2.5m to Y = +2.5m
      SimBalloon fr;
      fr.id = 10;
      fr.pos = vec(6.0, -2.5, 1.0);
      fr.vel = vec(0, 0.9, 0); // Drifts across line of fire
      fr.acc = vec(0, 0, 0);
      fr.isFriendly = true;
      fr.color = Color{41, 182, 246, 255};
      state.simBalloons.push_back(fr);
      break;
   }
   case ScenarioMode::MULTI_TARGET_PRIORITY: {
      state.announcementText = "SCENARIO 7: MULTI-TARGET GREEDY THREAT PRIORITY SORTING";
      state.announcementTimer = 3.0f;
      // Target B: Close, aggressive dive bomber (higher threat!) -> will be engaged 1st!
      SimBalloon b;
      b.id = 21;
      b.pos = vec(7.5, 2.5, 2.0);
      b.vel = vec(-3.0, 0, 0);
      b.acc = vec(0, 0, 0);
      b.isFriendly = false;
      b.color = Color{244, 67, 54, 255};
      state.simBalloons.push_back(b);

      // Target A: Far, slow moving -> will be engaged 2nd after Target B is destroyed!
      SimBalloon a;
      a.id = 20;
      a.pos = vec(17.0, -3.5, 1.0);
      a.vel = vec(0, 0, 0);
      a.acc = vec(0, 0, 0);
      a.isFriendly = false;
      a.color = Color{244, 67, 54, 255};
      state.simBalloons.push_back(a);

      // Friendly unit in the area
      SimBalloon fr;
      fr.id = 22;
      fr.pos = vec(9.0, -1.0, 0.5);
      fr.vel = vec(0, 0, 0);
      fr.acc = vec(0, 0, 0);
      fr.isFriendly = true;
      fr.color = Color{41, 182, 246, 255};
      state.simBalloons.push_back(fr);
      break;
   }
   case ScenarioMode::RANDOM_SWARM: {
      state.announcementText = "SCENARIO 8: RANDOM 3D HOSTILE SWARM (SEQUENTIAL ENGAGEMENT)";
      state.announcementTimer = 3.0f;
      for (uint8_t id = 1; id <= 8; ++id) {
         SimBalloon b;
         b.id = id;
         double dist = 6.0 + (rand() % 120) / 10.0;
         double y = -5.5 + (rand() % 110) / 10.0;
         double z = 0.8 + (rand() % 35) / 10.0;
         double vy = -2.0 + (rand() % 40) / 10.0;
         b.pos = vec(dist, y, z);
         b.vel = vec(0, vy, 0);
         b.acc = vec(0, 0, 0);
         b.isFriendly = (id % 3 == 0); // 1 friendly every 2 hostiles
         b.color = b.isFriendly ? Color{41, 182, 246, 255} : Color{244, 67, 54, 255};
         state.simBalloons.push_back(b);
      }
      break;
   }
   }

   // PRIME THE BACKEND FILTERS:
   // Pre-populate 5 historical frames spaced in time so Savitzky-Golay filters activate immediately
   for (int i = 0; i < HISTORY_FRAME_COUNTS; ++i) {
      double tOffset = (i - (HISTORY_FRAME_COUNTS - 1)) * 0.033;
      ImageFrame* f = sys.pushFrame(vec(0, 0, 0), ++state.frameCounter);
      for (const auto& b : state.simBalloons) {
         if (b.isAlive) {
            Object o(b.pos + b.vel * tOffset, b.vel, b.acc, b.isFriendly);
            o.ID = b.id;
            o.isAlive = true;
            f->detectedObjects[b.id] = o;
         }
      }
   }
   sys.updateKinematics();
}

// Helper to load high-res GPU font with bilinear filtering from system font candidates
Font LoadGPUFont(const std::vector<const char*>& paths, int size) {
   for (const char* path : paths) {
      if (FileExists(path)) {
         Font f = LoadFontEx(path, size, nullptr, 0);
         if (f.texture.id > 0) {
            SetTextureFilter(f.texture, TEXTURE_FILTER_BILINEAR);
            return f;
         }
      }
   }
   return GetFontDefault();
}

// Wrapper for GPU-accelerated text rendering with custom font
void DrawTextGPU(Font font, const std::string& text, float x, float y, float size, Color color, float spacing = 1.0f) {
   DrawTextEx(font, text.c_str(), Vector2{x, y}, size, spacing, color);
}

int main() {
   srand(static_cast<unsigned int>(time(nullptr)));

   // Set window flags: Fullscreen, Anti-aliasing, V-Sync, Resizable
   SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);

   int monitor = GetCurrentMonitor();
   int monitorWidth = GetMonitorWidth(monitor);
   int monitorHeight = GetMonitorHeight(monitor);

   if (monitorWidth <= 0 || monitorHeight <= 0) {
      monitorWidth = 1920;
      monitorHeight = 1080;
   }

   InitWindow(monitorWidth, monitorHeight, "Teknofest Air Defense - 3D Backend Visualizer");
   if (!IsWindowFullscreen()) {
      ToggleFullscreen();
   }
   SetTargetFPS(60);

   // Load high-resolution, GPU-accelerated Arial-equivalent fonts (GNU FreeSans / NotoSans / DejaVuSans)
   const std::vector<const char*> boldFontCandidates = {
       "/usr/share/fonts/gnu-free/FreeSansBold.otf",
       "/usr/share/fonts/noto/NotoSans-Bold.ttf",
       "/usr/share/fonts/TTF/DejaVuSans-Bold.ttf"};
   const std::vector<const char*> regularFontCandidates = {
       "/usr/share/fonts/gnu-free/FreeSans.otf",
       "/usr/share/fonts/noto/NotoSans-Regular.ttf",
       "/usr/share/fonts/TTF/DejaVuSans.ttf"};

   Font fontBold = LoadGPUFont(boldFontCandidates, 64);
   Font fontReg = LoadGPUFont(regularFontCandidates, 56);

   // Compile GLSL 330 Fragment Shader for Tactical Scope post-processing
   Shader tacticalShader = LoadShaderFromMemory(nullptr, g_tacticalShaderFS);
   int locTime = GetShaderLocation(tacticalShader, "uTime");
   int locRes = GetShaderLocation(tacticalShader, "uResolution");
   int locMode = GetShaderLocation(tacticalShader, "uShaderMode");
   int locLaser = GetShaderLocation(tacticalShader, "uLaserIntensity");

   // Create high-res offscreen render target for the 3D scene
   int sw = GetScreenWidth();
   int sh = GetScreenHeight();
   RenderTexture2D targetTexture = LoadRenderTexture(sw, sh);

   // Instantiate Backend Hardware Subsystems
   Camera cam(vec(0, 0, 0));
   Engine leftYaw, rightYaw, pitch;
   Laser laser;
   Brain brain;
   Logger logger("./yazdan-backend/logs.txt");
   System sys(cam, leftYaw, rightYaw, pitch, laser, brain, logger);

   VisualizerState state;
   LoadScenario(state.mode, sys, state);

   // 3D Cameras
   RaylibCamera freeCam{};
   freeCam.position = Vector3{0.0f, 6.5f, -11.0f}; // Orbit perspective behind turret
   freeCam.target = Vector3{0.0f, 2.0f, 10.0f};    // Looking down engagement range
   freeCam.up = Vector3{0.0f, 1.0f, 0.0f};
   freeCam.fovy = 55.0f;
   freeCam.projection = CAMERA_PERSPECTIVE;

   RaylibCamera scopeCam{};
   scopeCam.position = Vector3{0.0f, 0.85f, 0.35f}; // Forward optic aperture position (clear of turret geometry)
   scopeCam.up = Vector3{0.0f, 1.0f, 0.0f};
   scopeCam.fovy = 50.0f;
   scopeCam.projection = CAMERA_PERSPECTIVE;

   while (!WindowShouldClose()) {
      float dt = GetFrameTime();
      if (dt > 0.05f)
         dt = 0.05f;

      // Handle window resize dynamically
      sw = GetScreenWidth();
      sh = GetScreenHeight();
      if (targetTexture.texture.width != sw || targetTexture.texture.height != sh) {
         UnloadRenderTexture(targetTexture);
         targetTexture = LoadRenderTexture(sw, sh);
      }

      // --- USER CONTROLS & HOTKEYS ---
      if (IsKeyPressed(KEY_F11) || IsKeyPressed(KEY_F)) {
         ToggleFullscreen();
      }
      if (IsKeyPressed(KEY_TAB)) {
         state.scopeView = !state.scopeView;
      }
      if (IsKeyPressed(KEY_SPACE)) {
         state.paused = !state.paused;
      }
      if (IsKeyPressed(KEY_M)) {
         state.shaderMode = (state.shaderMode + 1) % 4;
      }
      if (IsKeyPressed(KEY_R)) {
         LoadScenario(state.mode, sys, state);
      }
      if (IsKeyPressed(KEY_ONE)) {
         LoadScenario(ScenarioMode::STATIONARY_TARGET, sys, state);
      }
      if (IsKeyPressed(KEY_TWO)) {
         LoadScenario(ScenarioMode::CROSSING_FAST_TARGET, sys, state);
      }
      if (IsKeyPressed(KEY_THREE)) {
         LoadScenario(ScenarioMode::RADIAL_DIVE_BOMBER, sys, state);
      }
      if (IsKeyPressed(KEY_FOUR)) {
         LoadScenario(ScenarioMode::ACCELERATING_TARGET, sys, state);
      }
      if (IsKeyPressed(KEY_FIVE)) {
         LoadScenario(ScenarioMode::ZIG_ZAG_MANEUVER, sys, state);
      }
      if (IsKeyPressed(KEY_SIX)) {
         LoadScenario(ScenarioMode::FRIENDLY_IN_LINE_OF_FIRE, sys, state);
      }
      if (IsKeyPressed(KEY_SEVEN)) {
         LoadScenario(ScenarioMode::MULTI_TARGET_PRIORITY, sys, state);
      }
      if (IsKeyPressed(KEY_EIGHT)) {
         LoadScenario(ScenarioMode::RANDOM_SWARM, sys, state);
      }
      if (IsKeyPressed(KEY_C)) {
         // Dynamically spawn extra balloon into active scenario
         SimBalloon b;
         b.id = ++state.nextId;
         double dist = 7.0 + (rand() % 90) / 10.0;
         double y = -4.5 + (rand() % 90) / 10.0;
         double z = 1.0 + (rand() % 30) / 10.0;
         double vy = -2.0 + (rand() % 40) / 10.0;
         b.pos = vec(dist, y, z);
         b.vel = vec(0, vy, 0);
         b.acc = vec(0, 0, 0);
         b.isFriendly = (rand() % 3 == 0);
         b.color = b.isFriendly ? Color{41, 182, 246, 255} : Color{244, 67, 54, 255};
         state.simBalloons.push_back(b);
      }

      // Mouse Orbit controls in Free 3D view
      if (!state.scopeView) {
         if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
            Vector2 delta = GetMouseDelta();
            freeCam.position.x += delta.x * 0.05f;
            freeCam.position.y -= delta.y * 0.05f;
         }
         float wheel = GetMouseWheelMove();
         if (wheel != 0) {
            freeCam.position.z += wheel * 1.2f;
         }
      }

      // Announcement banner timer countdown
      if (state.announcementTimer > 0.0f) {
         state.announcementTimer -= dt;
      }
      if (state.friendlyHitWarning > 0.0f) {
         state.friendlyHitWarning -= dt;
      }

      // --- DYNAMIC SIMULATION PHYSICS & SCENARIO PLAYBACK ---
      if (!state.paused) {
         state.simTime += dt;

         for (auto& b : state.simBalloons) {
            if (!b.isAlive) {
               // Dead balloons descend slowly to the ground ONCE, then stay stationary forever
               if (b.pos.z > 0.25) {
                  b.pos.z -= 0.8 * dt;
                  if (b.pos.z <= 0.25) {
                     b.pos.z = 0.25;
                     b.vel = vec(0, 0, 0);
                     b.acc = vec(0, 0, 0);
                  }
               }
               continue;
            }

            // Alive scenario kinematics
            if (state.mode == ScenarioMode::STATIONARY_TARGET) {
               b.pos.z = 1.5 + 0.10 * std::sin(state.simTime * 2.5); // Gentle buoyant bob
            }
            else if (state.mode == ScenarioMode::CROSSING_FAST_TARGET) {
               b.pos += b.vel * dt;
               if (b.pos.y > 6.5) {
                  b.vel.y = -std::abs(b.vel.y);
               }
               if (b.pos.y < -6.5) {
                  b.vel.y = std::abs(b.vel.y);
               }
            }
            else if (state.mode == ScenarioMode::RADIAL_DIVE_BOMBER) {
               b.pos += b.vel * dt;
               if (b.pos.x <= 2.5) {
                  b.pos = vec(22.0, 0.0, 5.0); // Reset dive pass
               }
            }
            else if (state.mode == ScenarioMode::ACCELERATING_TARGET) {
               b.vel += b.acc * dt;
               b.pos += b.vel * dt;
               if (b.pos.y > 6.5 || b.pos.y < -6.5) {
                  b.vel.y = -b.vel.y;
                  b.acc.y = -b.acc.y;
               }
               if (b.pos.z > 5.0 || b.pos.z < 0.8) {
                  b.vel.z = -b.vel.z;
                  b.acc.z = -b.acc.z;
               }
            }
            else if (state.mode == ScenarioMode::ZIG_ZAG_MANEUVER) {
               b.pos.x = 11.0 + 1.2 * std::sin(state.simTime * 1.2);
               b.vel.y = 4.2 * std::cos(state.simTime * 2.6);
               b.pos.y += b.vel.y * dt;
            }
            else if (state.mode == ScenarioMode::FRIENDLY_IN_LINE_OF_FIRE) {
               if (b.isFriendly) {
                  // Allow manual flight control with Left/Right keys OR dynamic patrol across line of fire
                  if (IsKeyDown(KEY_LEFT)) {
                     b.pos.y -= 2.5 * dt;
                  }
                  else if (IsKeyDown(KEY_RIGHT)) {
                     b.pos.y += 2.5 * dt;
                  }
                  else {
                     b.pos += b.vel * dt;
                     if (b.pos.y > 2.5) {
                        b.vel.y = -std::abs(b.vel.y);
                     }
                     if (b.pos.y < -2.5) {
                        b.vel.y = std::abs(b.vel.y);
                     }
                  }
               }
            }
            else if (state.mode == ScenarioMode::MULTI_TARGET_PRIORITY) {
               b.pos += b.vel * dt;
               if (b.pos.x < 3.0 && b.vel.x < 0) {
                  b.pos.x = 7.5;
               }
            }
            else if (state.mode == ScenarioMode::RANDOM_SWARM) {
               b.pos += b.vel * dt;
               if (b.pos.y > 7.5 || b.pos.y < -7.5)
                  b.vel.y = -b.vel.y;
               if (b.pos.x > 19.0 || b.pos.x < 5.0)
                  b.vel.x = -b.vel.x;
               if (b.pos.z > 5.2 || b.pos.z < 0.8)
                  b.vel.z = -b.vel.z;
            }
         }

         // Update 3D explosion particles
         for (auto& p : state.particles) {
            p.pos.x += p.vel.x * dt;
            p.pos.y += p.vel.y * dt;
            p.pos.z += p.vel.z * dt;
            p.vel.y -= 9.8f * dt;
            p.life -= dt;
            p.alpha = std::clamp(p.life / 0.8f, 0.0f, 1.0f);
         }
         state.particles.erase(std::remove_if(state.particles.begin(), state.particles.end(), [](const Particle& p) { return p.life <= 0.0f; }), state.particles.end());

         // Feed objects into the next hardware camera image frame (passing true isAlive state)
         ImageFrame* nextFrame = sys.pushFrame(vec(0, 0, 0), ++state.frameCounter);
         for (const auto& b : state.simBalloons) {
            Object o(b.pos, b.vel, b.acc, b.isFriendly);
            o.ID = b.id;
            o.isAlive = b.isAlive;
            nextFrame->detectedObjects[b.id] = o;
         }

         // Advance closed-loop targeting and fire control backend
         sys.tick(dt);

         // Compute 3D Aim Target
         double yawRad = toRadians(sys.yawAngle);
         double pitchRad = toRadians(sys.pitchAngle);
         float lookX = static_cast<float>(std::sin(yawRad) * std::cos(pitchRad));
         float lookY = static_cast<float>(std::sin(pitchRad));
         float lookZ = static_cast<float>(std::cos(yawRad) * std::cos(pitchRad));
         vec aimTargetTk = vec(lookZ * 18.0, lookX * 18.0, lookY * 18.0);

         auto& tracked = sys.getTrackedObjects();
         Target currentBest = brain.pickBestTarget(tracked, cam.CamCenterPos);
         if (currentBest) {
            aimTargetTk = brain.CalculateInterceptionPoint(*currentBest, cam.CamCenterPos);
         }

         // Check if a friendly balloon is currently obstructing the line of fire
         state.friendlyObstructed = false;
         if (currentBest) {
            if (!brain.isShotSafe(aimTargetTk, MAX_LASER_RADIUS, tracked, cam.CamCenterPos)) {
               state.friendlyObstructed = true;
            }
         }

         // --- SYNC TARGET DESTRUCTION AND WEAPON DAMAGE ACROSS ALL BALLOONS ---
         vec laserOriginTk = cam.CamCenterPos + CAMERA_LASER_OFFSET;

         for (auto& b : state.simBalloons) {
            if (!b.isAlive)
               continue;

            auto it = tracked.find(b.id);

            // Sync health directly from backend timeUnderFire
            if (it != tracked.end()) {
               b.health = std::max(0.0, 1.0 - (it->second.timeUnderFire / TIME_UNDER_FIRE));
            }

            // Also check physical laser beam intersection if weapon is active
            if (laser.isOn) {
               double distToBeam = DistancePointToSegment(b.pos, laserOriginTk, aimTargetTk);
               if (distToBeam <= 0.50) { // Inside balloon physical radius
                  b.health -= dt / TIME_UNDER_FIRE;
                  if (b.isFriendly) {
                     state.friendlyHitWarning = 0.5f;
                  }
               }
            }

            // DESTRUCTION TRIGGER:
            // If backend marked dead, OR timeUnderFire reached 3.0s, OR health reached 0:
            bool backendDead = (it != tracked.end()) && (!it->second.isAlive || it->second.timeUnderFire >= TIME_UNDER_FIRE);
            if (backendDead || b.health <= 0.0) {
               b.isAlive = false;
               b.health = 0.0;
               b.color = Color{95, 100, 110, 255}; // Charred metallic gray!
               if (it != tracked.end()) {
                  it->second.isAlive = false;       // Ensure backend immediately marks dead!
               }

               if (b.isFriendly) {
                  state.friendlyCasualties++;
                  SpawnExplosion(state, toRaylib(b.pos), SKYBLUE);
                  state.announcementText = "CRITICAL: FRIENDLY ASSET DESTROYED BY LASER!";
                  state.announcementTimer = 3.5f;
               }
               else {
                  state.targetsNeutralized++;
                  SpawnExplosion(state, toRaylib(b.pos), RED);
               }
            }
         }
      }

      // --- COMPUTE 3D TURRET & CAMERA DIRECTIONS ---
      double yawRad = toRadians(sys.yawAngle);
      double pitchRad = toRadians(sys.pitchAngle);
      float lookX = static_cast<float>(std::sin(yawRad) * std::cos(pitchRad));
      float lookY = static_cast<float>(std::sin(pitchRad));
      float lookZ = static_cast<float>(std::cos(yawRad) * std::cos(pitchRad));

      scopeCam.target = Vector3{scopeCam.position.x + lookX * 25.0f,
                                scopeCam.position.y + lookY * 25.0f,
                                scopeCam.position.z + lookZ * 25.0f};

      RaylibCamera& currentCam = state.scopeView ? scopeCam : freeCam;

      // Current best locked target and aim interception point
      auto& tracked = sys.getTrackedObjects();
      Target currentBest = brain.pickBestTarget(tracked, cam.CamCenterPos);
      uint8_t bestId = currentBest ? currentBest->ID : 0;

      Vector3 aimPoint = {lookX * 18.0f, lookY * 18.0f, lookZ * 18.0f};
      if (currentBest) {
         vec intercept = brain.CalculateInterceptionPoint(*currentBest, cam.CamCenterPos);
         aimPoint = toRaylib(intercept);
      }

      // --- PASS 1: RENDER 3D SCENE TO TEXTURE ---
      BeginTextureMode(targetTexture);
      ClearBackground(Color{12, 18, 28, 255}); // Slate tactical blue

      BeginMode3D(currentCam);

      // Tactical Ground Grid
      DrawGrid(50, 1.0f);

      // ONLY draw 20m perimeter ring in Free 3D Orbit view (NEVER in 1st person scope to avoid blocking the view!)
      if (!state.scopeView) {
         for (int a = 0; a < 360; a += 6) {
            float r1 = toRadians(a);
            float r2 = toRadians(a + 6);
            Vector3 p1 = {static_cast<float>(std::sin(r1) * MAX_DISTANCE), 0, static_cast<float>(std::cos(r1) * MAX_DISTANCE)};
            Vector3 p2 = {static_cast<float>(std::sin(r2) * MAX_DISTANCE), 0, static_cast<float>(std::cos(r2) * MAX_DISTANCE)};
            DrawLine3D(p1, p2, ColorAlpha(RED, 0.35f));
         }

         // Draw Turret mesh in orbit view
         DrawTurret(sys.yawAngle, sys.pitchAngle, laser.isOn, aimPoint);
      }
      else {
         // In 1st person scope view: draw laser beam emerging forward without turret body obscuring view
         if (laser.isOn) {
            vec laserOffsetTk = CAMERA_LASER_OFFSET;
            Vector3 laserEmitter = {scopeCam.position.x + static_cast<float>(laserOffsetTk.y),
                                    scopeCam.position.y - 0.1f + static_cast<float>(laserOffsetTk.z),
                                    scopeCam.position.z + static_cast<float>(laserOffsetTk.x)};
            DrawLine3D(laserEmitter, aimPoint, RED);
            DrawLine3D(laserEmitter, aimPoint, WHITE);
            DrawSphere(aimPoint, 0.10f, RED);
            DrawSphere(aimPoint, 0.04f, WHITE);
         }
      }

      // Render Balloons
      for (const auto& b : state.simBalloons) {
         Vector3 rPos = toRaylib(b.pos);
         bool isTracked = (b.id == bestId && b.isAlive);

         DrawBalloon(rPos, b.color, isTracked, b.health, b.isAlive);

         // 3D Velocity vector indicator for alive balloons
         if (b.isAlive) {
            Vector3 velEnd = {rPos.x + static_cast<float>(b.vel.y),
                              rPos.y + static_cast<float>(b.vel.z),
                              rPos.z + static_cast<float>(b.vel.x)};
            DrawLine3D(rPos, velEnd, YELLOW);
         }
      }

      // Render 3D Explosion Particles
      for (const auto& p : state.particles) {
         DrawSphere(p.pos, 0.09f, ColorAlpha(p.color, p.alpha));
      }

      EndMode3D();
      EndTextureMode();

      // --- PASS 2: COMPOSITE SCENE THROUGH GPU POST-PROCESSING SHADER ---
      BeginDrawing();
      ClearBackground(BLACK);

      BeginShaderMode(tacticalShader);
      float simTimeSec = state.simTime;
      float resVec[2] = {static_cast<float>(sw), static_cast<float>(sh)};
      float laserIntensity = laser.isOn ? 1.0f : 0.0f;

      SetShaderValue(tacticalShader, locTime, &simTimeSec, SHADER_UNIFORM_FLOAT);
      SetShaderValue(tacticalShader, locRes, resVec, SHADER_UNIFORM_VEC2);
      SetShaderValue(tacticalShader, locMode, &state.shaderMode, SHADER_UNIFORM_INT);
      SetShaderValue(tacticalShader, locLaser, &laserIntensity, SHADER_UNIFORM_FLOAT);

      // Draw the offscreen texture flipped vertically onto full screen
      DrawTextureRec(targetTexture.texture,
                     Rectangle{0, 0, static_cast<float>(targetTexture.texture.width), -static_cast<float>(targetTexture.texture.height)},
                     Vector2{0, 0}, WHITE);
      EndShaderMode();

      // --- PASS 3: 2D VECTOR HUD & GPU ACCELERATED TYPOGRAPHY ---

      // 1. Sleek Tactical Reticles (Clean tick marks, zero blocking rings)
      int cx = sw / 2;
      int cy = sh / 2;

      // Camera Optical Center Crosshair [+]
      DrawLine(cx - 18, cy, cx + 18, cy, ColorAlpha(GREEN, 0.9f));
      DrawLine(cx, cy - 18, cx, cy + 18, ColorAlpha(GREEN, 0.9f));
      DrawTextGPU(fontBold, "CAM [+]", cx + 14, cy - 20, 18, GREEN);

      // Laser Offset Reticle [X] (Parallax adjusted for CAMERA_LASER_OFFSET)
      Vector2 laserScreenPos = GetWorldToScreen(aimPoint, currentCam);
      if (laserScreenPos.x >= 0 && laserScreenPos.x <= sw &&
          laserScreenPos.y >= 0 && laserScreenPos.y <= sh) {
         int lx = static_cast<int>(laserScreenPos.x);
         int ly = static_cast<int>(laserScreenPos.y);

         Color laserCol = laser.isOn ? RED : ORANGE;
         DrawLine(lx - 12, ly - 12, lx + 12, ly + 12, laserCol);
         DrawLine(lx - 12, ly + 12, lx + 12, ly - 12, laserCol);
         DrawTextGPU(fontBold, "LASER [x]", lx + 16, ly - 10, 18, laserCol);
      }

      // 2. High-Visibility Floating 3D Balloon Information Cards (Displaying ID and Priority Threat Score)
      for (const auto& b : state.simBalloons) {
         Vector3 rPos = toRaylib(b.pos);

         // Skip if behind camera
         Vector3 toTarget = {rPos.x - currentCam.position.x, rPos.y - currentCam.position.y, rPos.z - currentCam.position.z};
         Vector3 forward = {currentCam.target.x - currentCam.position.x, currentCam.target.y - currentCam.position.y, currentCam.target.z - currentCam.position.z};
         float dot = toTarget.x * forward.x + toTarget.y * forward.y + toTarget.z * forward.z;
         if (dot <= 0.2f)
            continue;

         Vector2 tagPos = GetWorldToScreen(Vector3{rPos.x, rPos.y + 0.90f, rPos.z}, currentCam);

         if (tagPos.x > 80 && tagPos.x < sw - 120 && tagPos.y > 80 && tagPos.y < sh - 40) {
            if (b.isAlive) {
               Object fakeObj(b.pos, b.vel, b.acc, b.isFriendly);
               fakeObj.ID = b.id;
               double priorityScore = brain.EvaluateThreatLevel(fakeObj, cam.CamCenterPos);

               std::stringstream ssTitle, ssSub;
               bool isLocked = (b.id == bestId);

               if (b.isFriendly) {
                  ssTitle << "ID: " << (int)b.id << "  [FRIENDLY]";
                  ssSub << "Dist: " << std::fixed << std::setprecision(1) << b.pos.distanceTo(cam.CamCenterPos) << "m  HP: "
                        << static_cast<int>(b.health * 100) << "%";
               }
               else {
                  if (isLocked) {
                     ssTitle << ">> ID: " << (int)b.id << "  PRIORITY: " << std::fixed << std::setprecision(1) << priorityScore << " <<";
                  }
                  else {
                     ssTitle << "ID: " << (int)b.id << "  PRIORITY: " << std::fixed << std::setprecision(1) << priorityScore;
                  }
                  ssSub << "Dist: " << std::fixed << std::setprecision(1) << b.pos.distanceTo(cam.CamCenterPos) << "m  HP: "
                        << static_cast<int>(b.health * 100) << "%";
               }

               std::string titleStr = ssTitle.str();
               std::string subStr = ssSub.str();

               Vector2 titleSize = MeasureTextEx(fontBold, titleStr.c_str(), 19, 1.0f);
               Vector2 subSize = MeasureTextEx(fontReg, subStr.c_str(), 15, 1.0f);
               float boxW = std::max(titleSize.x, subSize.x) + 24;
               float boxH = titleSize.y + subSize.y + 12;

               float bx = tagPos.x - boxW / 2;
               float by = tagPos.y - boxH;

               Color borderCol = b.isFriendly ? SKYBLUE : (isLocked ? YELLOW : RED);
               Color titleCol = b.isFriendly ? SKYBLUE : (isLocked ? YELLOW : WHITE);

               DrawRectangle(bx, by, boxW, boxH, ColorAlpha(BLACK, 0.82f));
               DrawRectangleLines(bx, by, boxW, boxH, borderCol);

               DrawTextGPU(fontBold, titleStr, bx + 12, by + 4, 19, titleCol);
               DrawTextGPU(fontReg, subStr, bx + 12, by + 6 + titleSize.y, 15, LIGHTGRAY);
            }
            else {
               // Destroyed balloon tag
               std::stringstream ss;
               ss << "ID: " << (int)b.id << "  [DESTROYED]";
               std::string tagStr = ss.str();
               Vector2 tagSize = MeasureTextEx(fontBold, tagStr.c_str(), 16, 1.0f);
               float bx = tagPos.x - (tagSize.x + 16) / 2;
               float by = tagPos.y - (tagSize.y + 8);

               DrawRectangle(bx, by, tagSize.x + 16, tagSize.y + 8, ColorAlpha(BLACK, 0.70f));
               DrawRectangleLines(bx, by, tagSize.x + 16, tagSize.y + 8, DARKGRAY);
               DrawTextGPU(fontBold, tagStr, bx + 8, by + 4, 16, Color{150, 155, 165, 255});
            }
         }
      }

      // 3. Top Command & Control Navigation Bar
      DrawRectangle(0, 0, sw, 84, ColorAlpha(BLACK, 0.85f));
      DrawLine(0, 84, sw, 84, DARKGRAY);

      DrawTextGPU(fontBold, "TEKNOFEST AIR DEFENSE C2 - 3D TACTICAL TESTBENCH", 25, 10, 24, GOLD);

      std::string scenarioKeys = "SCENARIOS: [1] Stationary  [2] Crossing Fast  [3] Dive Bomber  [4] Accelerating  [5] Zig-Zag  [6] Friendly Safe  [7] Multi-Priority  [8] Swarm";
      DrawTextGPU(fontBold, scenarioKeys, 25, 38, 18, RAYWHITE);

      std::string controlKeys = "[TAB] View: Orbit/Scope  |  [M] Shader Mode  |  [F11/F] Fullscreen  |  [SPACE] Pause  |  [C] Spawn  |  [R] Reset  |  [<-/->] Steer Friendly";
      DrawTextGPU(fontReg, controlKeys, 25, 60, 16, LIGHTGRAY);

      // Top Right: Target Neutralized Tally & Friendly Casualties
      std::stringstream sTally;
      sTally << "NEUTRALIZED: " << state.targetsNeutralized;
      if (state.friendlyCasualties > 0) {
         sTally << "  |  FRIENDLY CASUALTIES: " << state.friendlyCasualties;
      }
      DrawTextGPU(fontBold, sTally.str(), sw - (state.friendlyCasualties > 0 ? 560 : 280), 22, 22, state.friendlyCasualties > 0 ? RED : GREEN);

      // 4. Left Telemetry & Kinematics Panel
      int panelW = 430;
      int panelH = 430;
      DrawRectangle(25, 105, panelW, panelH, ColorAlpha(BLACK, 0.80f));
      DrawRectangleLines(25, 105, panelW, panelH, DARKGRAY);

      DrawTextGPU(fontBold, "TURRET SYSTEM TELEMETRY", 45, 120, 22, YELLOW);

      std::stringstream sMode;
      sMode << "Active Scenario: ";
      switch (state.mode) {
      case ScenarioMode::STATIONARY_TARGET:
         sMode << "1. Stationary Target";
         break;
      case ScenarioMode::CROSSING_FAST_TARGET:
         sMode << "2. Fast Crossing Target";
         break;
      case ScenarioMode::RADIAL_DIVE_BOMBER:
         sMode << "3. Radial Dive Bomber";
         break;
      case ScenarioMode::ACCELERATING_TARGET:
         sMode << "4. Accelerating Target";
         break;
      case ScenarioMode::ZIG_ZAG_MANEUVER:
         sMode << "5. Evasive Zig-Zag Target";
         break;
      case ScenarioMode::FRIENDLY_IN_LINE_OF_FIRE:
         sMode << "6. Friendly Safe (Hold Fire)";
         break;
      case ScenarioMode::MULTI_TARGET_PRIORITY:
         sMode << "7. Multi-Target Threat Priority";
         break;
      case ScenarioMode::RANDOM_SWARM:
         sMode << "8. Dynamic 3D Swarm";
         break;
      }
      DrawTextGPU(fontBold, sMode.str(), 45, 150, 19, SKYBLUE);

      std::stringstream sYaw, sPitch, sVelY, sVelP;
      sYaw << "Yaw Position:     " << std::fixed << std::setprecision(2) << sys.yawAngle << " deg";
      sPitch << "Pitch Position:   " << std::fixed << std::setprecision(2) << sys.pitchAngle << " deg";
      sVelY << "Yaw Motor Rate:   " << std::fixed << std::setprecision(1) << leftYaw.velocity.x << " deg/s";
      sVelP << "Pitch Motor Rate: " << std::fixed << std::setprecision(1) << pitch.velocity.y << " deg/s";

      DrawTextGPU(fontBold, sYaw.str(), 45, 180, 20, WHITE);
      DrawTextGPU(fontBold, sPitch.str(), 45, 208, 20, WHITE);
      DrawTextGPU(fontReg, sVelY.str(), 45, 238, 19, LIGHTGRAY);
      DrawTextGPU(fontReg, sVelP.str(), 45, 264, 19, LIGHTGRAY);

      // Weapon State Indicator Badge
      if (laser.isOn) {
         DrawRectangle(45, 298, 250, 34, ColorAlpha(RED, 0.85f));
         DrawRectangleLines(45, 298, 250, 34, YELLOW);
         DrawTextGPU(fontBold, "LASER ACTIVE [FIRING]", 58, 304, 19, WHITE);
      }
      else if (state.friendlyObstructed) {
         DrawRectangle(45, 298, 250, 34, ColorAlpha(ORANGE, 0.85f));
         DrawRectangleLines(45, 298, 250, 34, YELLOW);
         DrawTextGPU(fontBold, "HOLD FIRE: SAFEGUARD", 52, 304, 19, BLACK);
      }
      else {
         DrawRectangle(45, 298, 250, 34, ColorAlpha(DARKGREEN, 0.85f));
         DrawRectangleLines(45, 298, 250, 34, LIME);
         DrawTextGPU(fontBold, "LASER STANDBY [SEEK]", 56, 304, 19, WHITE);
      }

      // Camera Mode and Shader indicators
      std::string viewStr = state.scopeView ? "Optic: 1st-Person Scope" : "Optic: 3D Orbit (R-Click Orbit)";
      DrawTextGPU(fontBold, viewStr, 45, 345, 18, GOLD);

      std::string shaderNames[] = {"Shader: Tactical HUD", "Shader: FLIR Thermal IR", "Shader: Night Vision (NVG)", "Shader: Clean Raw"};
      DrawTextGPU(fontBold, shaderNames[state.shaderMode], 45, 372, 18, LIME);

      // Kill Lock Progress Bar
      if (currentBest && currentBest->isAlive && currentBest->timeUnderFire > 0) {
         float progress = static_cast<float>(currentBest->timeUnderFire / TIME_UNDER_FIRE);
         DrawRectangle(45, 405, 350, 16, DARKGRAY);
         DrawRectangle(45, 405, static_cast<int>(350 * std::clamp(progress, 0.0f, 1.0f)), 16, RED);
         DrawRectangleLines(45, 405, 350, 16, WHITE);
         DrawTextGPU(fontBold, "Sustained Burn Progress", 45, 424, 15, LIGHTGRAY);
      }

      // 5. Right Threat Matrix Panel
      int rosterW = 490;
      int rosterH = 430;
      DrawRectangle(sw - rosterW - 25, 105, rosterW, rosterH, ColorAlpha(BLACK, 0.80f));
      DrawRectangleLines(sw - rosterW - 25, 105, rosterW, rosterH, DARKGRAY);

      DrawTextGPU(fontBold, "LIVE TARGET MATRIX & THREAT ROSTER", sw - rosterW - 5, 120, 22, YELLOW);

      int itemY = 155;
      int displayed = 0;
      for (const auto& b : state.simBalloons) {
         if (displayed >= 5)
            break;

         if (b.isAlive) {
            Object fakeObj(b.pos, b.vel, b.acc, b.isFriendly);
            fakeObj.ID = b.id;
            double score = brain.EvaluateThreatLevel(fakeObj, cam.CamCenterPos);
            bool isLocked = (b.id == bestId);

            if (isLocked) {
               DrawRectangle(sw - rosterW - 20, itemY - 2, rosterW - 10, 48, ColorAlpha(RED, 0.35f));
               DrawRectangleLines(sw - rosterW - 20, itemY - 2, rosterW - 10, 48, RED);
            }

            std::stringstream sObj, sDyn;
            sObj << (isLocked ? ">> " : "   ")
                 << "ID:" << (int)b.id << " " << (b.isFriendly ? "[FRIENDLY]" : "[HOSTILE]")
                 << " Priority:" << std::fixed << std::setprecision(1) << score
                 << " Dist:" << std::fixed << std::setprecision(1) << b.pos.distanceTo(cam.CamCenterPos) << "m";

            sDyn << "   Pos:(" << std::fixed << std::setprecision(1) << b.pos.x << "," << b.pos.y << "," << b.pos.z << ")"
                 << " Vel:(" << b.vel.x << "," << b.vel.y << "," << b.vel.z << ") HP:" << static_cast<int>(b.health * 100) << "%";

            Color rowColor = isLocked ? YELLOW : (b.isFriendly ? SKYBLUE : WHITE);
            DrawTextGPU(fontBold, sObj.str(), sw - rosterW - 10, itemY, 18, rowColor);
            DrawTextGPU(fontReg, sDyn.str(), sw - rosterW - 10, itemY + 22, 16, LIGHTGRAY);
         }
         else {
            // Destroyed targets shown in muted gray
            std::stringstream sDead;
            sDead << "   ID:" << (int)b.id << " [DESTROYED / CHARRED GRAY]";
            DrawTextGPU(fontReg, sDead.str(), sw - rosterW - 10, itemY + 8, 17, Color{130, 135, 145, 255});
         }

         itemY += 52;
         displayed++;
      }

      // 6. Dynamic Central Announcement Banner
      if (state.friendlyHitWarning > 0.0f) {
         std::string critText = "CRITICAL: FRIENDLY UNIT BURNING UNDER LASER BEAM!";
         Vector2 bannerSize = MeasureTextEx(fontBold, critText.c_str(), 24, 1.0f);
         int bx = (sw - bannerSize.x) / 2;
         int by = 96;
         DrawRectangle(bx - 20, by - 8, bannerSize.x + 40, bannerSize.y + 16, ColorAlpha(RED, 0.90f));
         DrawRectangleLines(bx - 20, by - 8, bannerSize.x + 40, bannerSize.y + 16, YELLOW);
         DrawTextGPU(fontBold, critText, bx, by, 24, WHITE);
      }
      else if (state.friendlyObstructed) {
         std::string warnText = "SAFETY INTERLOCK: FRIENDLY IN LINE OF FIRE - HOLDING WEAPON";
         Vector2 bannerSize = MeasureTextEx(fontBold, warnText.c_str(), 24, 1.0f);
         int bx = (sw - bannerSize.x) / 2;
         int by = 96;
         DrawRectangle(bx - 20, by - 8, bannerSize.x + 40, bannerSize.y + 16, ColorAlpha(BLACK, 0.85f));
         DrawRectangleLines(bx - 20, by - 8, bannerSize.x + 40, bannerSize.y + 16, ORANGE);
         DrawTextGPU(fontBold, warnText, bx, by, 24, YELLOW);
      }
      else if (state.announcementTimer > 0.0f) {
         float alpha = std::clamp(state.announcementTimer, 0.0f, 1.0f);
         Vector2 bannerSize = MeasureTextEx(fontBold, state.announcementText.c_str(), 26, 1.0f);
         int bx = (sw - bannerSize.x) / 2;
         int by = 100;
         DrawRectangle(bx - 20, by - 8, bannerSize.x + 40, bannerSize.y + 16, ColorAlpha(BLACK, 0.85f * alpha));
         DrawRectangleLines(bx - 20, by - 8, bannerSize.x + 40, bannerSize.y + 16, ColorAlpha(GOLD, alpha));
         DrawTextGPU(fontBold, state.announcementText, bx, by, 26, ColorAlpha(GOLD, alpha));
      }

      // FPS & Render Performance
      DrawFPS(sw - 120, sh - 36);

      EndDrawing();
   }

   // Cleanup resources
   UnloadRenderTexture(targetTexture);
   UnloadShader(tacticalShader);
   if (fontBold.texture.id > 0)
      UnloadFont(fontBold);
   if (fontReg.texture.id > 0)
      UnloadFont(fontReg);

   CloseWindow();
   return 0;
}
