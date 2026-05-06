#pragma once
#include <iostream>
#include <cmath>
#include <vector>
#include <utility>
#include <algorithm>

/*
	================================================
	| IMPORTANT : coorrdinates are in meters,      |
	| angles are in degrees, time is in seconds,   |
	| velocity is in m/s, acceleration is in m/s^2 |
	================================================
*/

extern const double TIME_UNDER_FIRE;
double toRadians(double degrees);
double toDegrees(double radians);
typedef struct Vector3D{
	 double x, y, z; // (x,y,z) coordinates
	 Vector3D(double x = 0, double y = 0, double z = 0) : x(x), y(y), z(z) {};
	 double operator[](size_t i) const;
	 double operator*(const Vector3D& other) const; // dot product
	 double operator *=(const Vector3D& other); // dot product assignment
	 Vector3D operator*(double scalar) const; // scalar multiplication
	 Vector3D operator+(const Vector3D& other) const; // vector addition 
	 Vector3D operator-(const Vector3D& other) const; // vector subtraction
	 Vector3D operator/(double scalar) const; // scalar division
	 Vector3D& operator+=(const Vector3D& other); // vector addition assignment
	 Vector3D& operator-=(const Vector3D& other); // vector subtraction assignment
	 Vector3D& operator*=(double scalar); // scalar multiplication assignment  
	 Vector3D& operator/=(double scalar); // scalar division assignment
	 Vector3D cross(const Vector3D& other) const; // cross product
	 double magnitude() const; // magnitude of the vector
	 double distanceTo(const Vector3D& other) const; // distance to another vector
	 double angleTo(const Vector3D& other) const; // angle to another vector in radians
	 double YawTo(const Vector3D& other) const; // yaw angle to another vector in degrees
	 double PitchTo(const Vector3D& other) const; // pitch angle to another vector in degrees
	friend std::ostream& operator<<(std::ostream& os, const Vector3D& vec);
	 ~Vector3D() = default;
} vec;
typedef struct Object{
	 vec pos, vel, acc; // position, velocity, acceleration
	 u_int8_t ID;
	 bool isFriendly, isAlive; 
	 double timeUnderFire=0.0;
	 Object(vec p, vec v = vec(0,0,0), vec a = vec(0,0,0), bool isFriendly = false) : 
	 pos(p), vel(v), acc(a), isFriendly(isFriendly), isAlive(true) {};
	friend std::ostream& operator<<(std::ostream& os, const Object& obj);
	~Object() = default;
} obj;
