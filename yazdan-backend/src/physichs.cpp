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
double Vector3D::magnitude() const {
   return std::sqrt(x * x + y * y + z * z);
}
double Vector3D::distanceTo(const Vector3D& other) const {
   return (*this - other).magnitude();
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
   return toDegrees(std::atan2(other[1] - this->y, other[0] - this->x));
}
double Vector3D::PitchTo(const Vector3D& other) const {
   return toDegrees(std::atan2(other[2] - this->z, std::hypot(other[0] - this->x, other[1] - this->y)));
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
