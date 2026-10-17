#include "../include/physichs.hpp"
#include <algorithm>
#include <cmath>
#include <iostream>
extern const double TIME_UNDER_FIRE;
double toRadians(double degrees) {
   return degrees * M_PI / 180.0;
}
double toDegrees(double radians) {
   return radians * 180.0 / M_PI;
}
double Vector3D::operator[](size_t i) const {
   switch (i) {
   case 0:
      return x;
   case 1:
      return y;
   case 2:
      return z;
   default:
      throw std::out_of_range("Index out of bounds");
   }
}
double Vector3D::operator[](size_t i) {
   switch (i) {
   case 0:
      return x;
   case 1:
      return y;
   case 2:
      return z;
   default:
      throw std::out_of_range("Index out of bounds");
   }
}
double Vector3D::operator*(const Vector3D& other) const {
   return x * other.x + y * other.y + z * other.z;
}
double Vector3D::operator*=(const Vector3D& other) {
   return (*this) * other;
}
Vector3D Vector3D::operator*(double scalar) const {
   return Vector3D(x * scalar, y * scalar, z * scalar);
}
Vector3D Vector3D::operator+(const Vector3D& other) const {
   return Vector3D(x + other.x, y + other.y, z + other.z);
}
Vector3D Vector3D::operator-(const Vector3D& other) const {
   return Vector3D(x - other.x, y - other.y, z - other.z);
}
Vector3D Vector3D::operator/(double scalar) const {
   return Vector3D(x / scalar, y / scalar, z / scalar);
}
Vector3D& Vector3D::operator+=(const Vector3D& other) {
   x += other.x;
   y += other.y;
   z += other.z;
   return *this;
}
Vector3D& Vector3D::operator-=(const Vector3D& other) {
   x -= other.x;
   y -= other.y;
   z -= other.z;
   return *this;
}
Vector3D& Vector3D::operator*=(double scalar) {
   x *= scalar;
   y *= scalar;
   z *= scalar;
   return *this;
}
Vector3D& Vector3D::operator/=(double scalar) {
   x /= scalar;
   y /= scalar;
   z /= scalar;
   return *this;
}
Vector3D Vector3D::cross(const Vector3D& other) const {
   return Vector3D(y * other.z - z * other.y, z * other.x - x * other.z, x * other.y - y * other.x);
}
Vector3D& Vector3D::CylindricalToCartesian(const Vector3D& other) {
   double cx = other.x * std::cos(other.y);
   double cy = other.x * std::sin(other.y);
   x = cx;
   y = cy;
   z = other.z;
   return *this;
}
Vector3D& Vector3D::CartesianToCylindrical(const Vector3D& other) {
   double radius = std::sqrt(other.x * other.x + other.y * other.y);
   double angle = std::atan2(other.y, other.x);
   x = radius;
   y = angle;
   z = other.z;
   return *this;
}
double Vector3D::magnitude() const {
   return std::sqrt(x * x + y * y + z * z);
}
double Vector3D::distanceTo(const Vector3D& other) const {
   double dx = x - other.x;
   double dy = y - other.y;
   double dz = z - other.z;
   return std::sqrt(dx * dx + dy * dy + dz * dz);
}
double Vector3D::angleTo(const Vector3D& other) const {
   double dot = (*this) * other;
   double magA = this->magnitude();
   double magB = other.magnitude();
   if (magA == 0 || magB == 0)
      return 0.0; // Avoid division by zero
   double cosAngle = dot / (magA * magB);
   // Clamp cosAngle to [-1, 1] to avoid numerical errors
   cosAngle = std::max(-1.0, std::min(1.0, cosAngle));
   return toDegrees(std::acos(cosAngle));
}
double Vector3D::YawTo(const Vector3D& other) const {
   return toDegrees(std::atan2(other.y - this->y, other.x - this->x));
}
double Vector3D::PitchTo(const Vector3D& other) const {
   double dx = other.x - this->x;
   double dy = other.y - this->y;
   double dz = other.z - this->z;
   return toDegrees(std::atan2(dz, std::sqrt(dx * dx + dy * dy)));
}
std::ostream& operator<<(std::ostream& os, const Vector3D& vec) {
   os << "(" << vec.x << ", " << vec.y << ", " << vec.z << ")";
   return os;
}
std::ostream& operator<<(std::ostream& os, const Object& obj) {
   os << "Position: " << obj.pos << ", Velocity: " << obj.vel << ", Acceleration: " << obj.acc
      << ", Friendly: " << (obj.isFriendly ? "Yes" : "No") << ", Alive: " << (obj.isAlive ? "Yes" : "No")
      << ", Time Under Fire: " << obj.timeUnderFire << " seconds";
   return os;
}

// --- FlatObjectMap Implementation ---
template <size_t MaxCapacity>
void FlatObjectMap<MaxCapacity>::Iterator::advance() {
   while (ptr != end && !ptr->active)
      ++ptr;
}

template <size_t MaxCapacity>
FlatObjectMap<MaxCapacity>::Iterator::Iterator(Entry* p, Entry* e) : ptr(p), end(e) {
   advance();
}

template <size_t MaxCapacity>
std::pair<uint8_t, Object>& FlatObjectMap<MaxCapacity>::Iterator::operator*() const {
   return ptr->kv;
}

template <size_t MaxCapacity>
std::pair<uint8_t, Object>* FlatObjectMap<MaxCapacity>::Iterator::operator->() const {
   return &ptr->kv;
}

template <size_t MaxCapacity>
typename FlatObjectMap<MaxCapacity>::Iterator& FlatObjectMap<MaxCapacity>::Iterator::operator++() {
   if (ptr != end) {
      ++ptr;
      advance();
   }
   return *this;
}

template <size_t MaxCapacity>
bool FlatObjectMap<MaxCapacity>::Iterator::operator!=(const Iterator& other) const {
   return ptr != other.ptr;
}

template <size_t MaxCapacity>
bool FlatObjectMap<MaxCapacity>::Iterator::operator==(const Iterator& other) const {
   return ptr == other.ptr;
}

template <size_t MaxCapacity>
void FlatObjectMap<MaxCapacity>::ConstIterator::advance() {
   while (ptr != end && !ptr->active)
      ++ptr;
}

template <size_t MaxCapacity>
FlatObjectMap<MaxCapacity>::ConstIterator::ConstIterator(const Entry* p, const Entry* e) : ptr(p), end(e) {
   advance();
}

template <size_t MaxCapacity>
const std::pair<uint8_t, Object>& FlatObjectMap<MaxCapacity>::ConstIterator::operator*() const {
   return ptr->kv;
}

template <size_t MaxCapacity>
const std::pair<uint8_t, Object>* FlatObjectMap<MaxCapacity>::ConstIterator::operator->() const {
   return &ptr->kv;
}

template <size_t MaxCapacity>
typename FlatObjectMap<MaxCapacity>::ConstIterator& FlatObjectMap<MaxCapacity>::ConstIterator::operator++() {
   if (ptr != end) {
      ++ptr;
      advance();
   }
   return *this;
}

template <size_t MaxCapacity>
bool FlatObjectMap<MaxCapacity>::ConstIterator::operator!=(const ConstIterator& other) const {
   return ptr != other.ptr;
}

template <size_t MaxCapacity>
bool FlatObjectMap<MaxCapacity>::ConstIterator::operator==(const ConstIterator& other) const {
   return ptr == other.ptr;
}

template <size_t MaxCapacity>
void FlatObjectMap<MaxCapacity>::insert_or_assign(uint8_t id, const Object& object) {
   for (size_t i = 0; i < MaxCapacity; ++i) {
      if (entries[i].active && entries[i].kv.first == id) {
         entries[i].kv.second = object;
         return;
      }
   }
   for (size_t i = 0; i < MaxCapacity; ++i) {
      if (!entries[i].active) {
         entries[i].kv.first = id;
         entries[i].kv.second = object;
         entries[i].active = true;
         activeCount++;
         return;
      }
   }
   throw std::out_of_range("FlatObjectMap capacity exceeded");
}

template <size_t MaxCapacity>
Object& FlatObjectMap<MaxCapacity>::operator[](uint8_t id) {
   for (size_t i = 0; i < MaxCapacity; ++i) {
      if (entries[i].active && entries[i].kv.first == id)
         return entries[i].kv.second;
   }
   for (size_t i = 0; i < MaxCapacity; ++i) {
      if (!entries[i].active) {
         entries[i].kv.first = id;
         entries[i].kv.second = Object();
         entries[i].active = true;
         activeCount++;
         return entries[i].kv.second;
      }
   }
   throw std::out_of_range("FlatObjectMap capacity exceeded");
}

template <size_t MaxCapacity>
size_t FlatObjectMap<MaxCapacity>::count(uint8_t id) const {
   for (size_t i = 0; i < MaxCapacity; ++i) {
      if (entries[i].active && entries[i].kv.first == id)
         return 1;
   }
   return 0;
}

template <size_t MaxCapacity>
Object& FlatObjectMap<MaxCapacity>::at(uint8_t id) {
   for (size_t i = 0; i < MaxCapacity; ++i) {
      if (entries[i].active && entries[i].kv.first == id)
         return entries[i].kv.second;
   }
   throw std::out_of_range("FlatObjectMap: id not found");
}

template <size_t MaxCapacity>
const Object& FlatObjectMap<MaxCapacity>::at(uint8_t id) const {
   for (size_t i = 0; i < MaxCapacity; ++i) {
      if (entries[i].active && entries[i].kv.first == id)
         return entries[i].kv.second;
   }
   throw std::out_of_range("FlatObjectMap: id not found");
}

template <size_t MaxCapacity>
void FlatObjectMap<MaxCapacity>::erase(uint8_t id) {
   for (size_t i = 0; i < MaxCapacity; ++i) {
      if (entries[i].active && entries[i].kv.first == id) {
         entries[i].active = false;
         activeCount--;
         return;
      }
   }
}

template <size_t MaxCapacity>
void FlatObjectMap<MaxCapacity>::clear() {
   for (size_t i = 0; i < MaxCapacity; ++i) {
      entries[i].active = false;
   }
   activeCount = 0;
}

template <size_t MaxCapacity>
bool FlatObjectMap<MaxCapacity>::empty() const {
   return activeCount == 0;
}

template <size_t MaxCapacity>
size_t FlatObjectMap<MaxCapacity>::size() const {
   return activeCount;
}

template <size_t MaxCapacity>
size_t FlatObjectMap<MaxCapacity>::capacity() const {
   return MaxCapacity;
}

template <size_t MaxCapacity>
typename FlatObjectMap<MaxCapacity>::Iterator FlatObjectMap<MaxCapacity>::begin() {
   return Iterator(entries.data(), entries.data() + MaxCapacity);
}

template <size_t MaxCapacity>
typename FlatObjectMap<MaxCapacity>::Iterator FlatObjectMap<MaxCapacity>::end() {
   return Iterator(entries.data() + MaxCapacity, entries.data() + MaxCapacity);
}

template <size_t MaxCapacity>
typename FlatObjectMap<MaxCapacity>::ConstIterator FlatObjectMap<MaxCapacity>::begin() const {
   return ConstIterator(entries.data(), entries.data() + MaxCapacity);
}

template <size_t MaxCapacity>
typename FlatObjectMap<MaxCapacity>::ConstIterator FlatObjectMap<MaxCapacity>::end() const {
   return ConstIterator(entries.data() + MaxCapacity, entries.data() + MaxCapacity);
}

template <size_t MaxCapacity>
typename FlatObjectMap<MaxCapacity>::ConstIterator FlatObjectMap<MaxCapacity>::cbegin() const {
   return begin();
}

template <size_t MaxCapacity>
typename FlatObjectMap<MaxCapacity>::ConstIterator FlatObjectMap<MaxCapacity>::cend() const {
   return end();
}

// Explicit template instantiations
template class FlatObjectMap<16>;
template class FlatObjectMap<32>;
template class FlatObjectMap<64>;
