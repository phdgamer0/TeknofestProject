#pragma once
#include <array>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <utility>

/*
        ================================================
        | IMPORTANT : coorrdinates are in meters,      |
        | angles are in degrees, time is in seconds,   |
        | velocity is in m/s, acceleration is in m/s^2 |
        ================================================
*/

double toRadians(double degrees);
double toDegrees(double radians);
typedef struct Vector3D {
   double x, y, z; // (x,y,z) coordinates
   constexpr Vector3D(double x = 0, double y = 0, double z = 0) : x(x), y(y), z(z) {};
   double operator[](size_t i) const;
   double operator[](size_t i);
   double operator*(const Vector3D& other) const;           // dot product
   double operator*=(const Vector3D& other);                // dot product assignment
   Vector3D operator*(double scalar) const;                 // scalar multiplication
   Vector3D operator+(const Vector3D& other) const;         // vector addition
   Vector3D operator-(const Vector3D& other) const;         // vector subtraction
   Vector3D operator/(double scalar) const;                 // scalar division
   Vector3D& operator+=(const Vector3D& other);             // vector addition assignment
   Vector3D& operator-=(const Vector3D& other);             // vector subtraction assignment
   Vector3D& operator*=(double scalar);                     // scalar multiplication assignment
   Vector3D& operator/=(double scalar);                     // scalar division assignment
   Vector3D cross(const Vector3D& other) const;             // cross product
   Vector3D& CylindricalToCartesian(const Vector3D& other); // convert cylindrical to cartesian
   Vector3D& CartesianToCylindrical(const Vector3D& other); // convert cartesian to cylindrical
   double magnitude() const;                                // magnitude of the vector
   double distanceTo(const Vector3D& other) const;          // distance to another vector
   double angleTo(const Vector3D& other) const;             // angle to another vector in radians
   double YawTo(const Vector3D& other) const;               // yaw angle to another vector in degrees
   double PitchTo(const Vector3D& other) const;             // pitch angle to another vector in degrees
   friend std::ostream& operator<<(std::ostream& os, const Vector3D& vec);
   ~Vector3D() = default;
} vec;
typedef struct Object {
   vec pos, vel, acc; // position, velocity, acceleration
   double timeUnderFire = 0.0;
   uint8_t ID{0};
   bool isFriendly{false}, isAlive{true}, isUrgent{false};
   uint8_t staleCount{0};
   Object(vec p = vec(0, 0, 0), vec v = vec(0, 0, 0), vec a = vec(0, 0, 0), bool isFriendly = false) : pos(p), vel(v), acc(a), isFriendly(isFriendly), isAlive(true), isUrgent(false), staleCount(0) {};
   friend std::ostream& operator<<(std::ostream& os, const Object& obj);
   ~Object() = default;
} obj;

// Fast, cache-local contiguous map for small ID key spaces (uint8_t ID, max 32 objects)
// Eliminates dynamic heap node allocations and CPU cache misses
template <size_t MaxCapacity = 32>
class FlatObjectMap {
 public:
   struct Entry {
      std::pair<uint8_t, Object> kv{0, Object()};
      bool active{false};
   };

   struct Iterator {
      Entry* ptr{nullptr};
      Entry* end{nullptr};
      void advance();
      Iterator(Entry* p, Entry* e);
      std::pair<uint8_t, Object>& operator*() const;
      std::pair<uint8_t, Object>* operator->() const;
      Iterator& operator++();
      bool operator!=(const Iterator& other) const;
      bool operator==(const Iterator& other) const;
   };

   struct ConstIterator {
      const Entry* ptr{nullptr};
      const Entry* end{nullptr};
      void advance();
      ConstIterator(const Entry* p, const Entry* e);
      const std::pair<uint8_t, Object>& operator*() const;
      const std::pair<uint8_t, Object>* operator->() const;
      ConstIterator& operator++();
      bool operator!=(const ConstIterator& other) const;
      bool operator==(const ConstIterator& other) const;
   };

 private:
   std::array<Entry, MaxCapacity> entries{};
   size_t activeCount{0};

 public:
   FlatObjectMap() = default;
   ~FlatObjectMap() = default;

   void insert_or_assign(uint8_t id, const Object& object);
   Object& operator[](uint8_t id);
   size_t count(uint8_t id) const;
   Object& at(uint8_t id);
   const Object& at(uint8_t id) const;
   void erase(uint8_t id);
   void clear();
   bool empty() const;
   size_t size() const;
   size_t capacity() const;

   Iterator begin();
   Iterator end();
   ConstIterator begin() const;
   ConstIterator end() const;
   ConstIterator cbegin() const;
   ConstIterator cend() const;
};
