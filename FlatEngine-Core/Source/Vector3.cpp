#include "Vector3.h"


namespace FlatEngine
{
	Vector3::Vector3()
	{
		x = 0;
		y = 0;
		z = 0;		
	}

	Vector3::Vector3(float xyzValue)
	{
		x = xyzValue;
		y = xyzValue;
		z = xyzValue;		
	}

	Vector3::Vector3(float xValue, float yValue, float zValue)
	{
		x = xValue;
		y = yValue;
		z = zValue;
	}

	Vector3::Vector3(const Vector3& toCopy)
	{
		x = toCopy.x;
		y = toCopy.y;
		z = toCopy.z;
	}

	Vector3::~Vector3()
	{
	}

	void Vector3::_xyz(float newX, float newY, float newZ)
	{
		x = newX;
		y = newY;
		z = newZ;
	}

	float Vector3::GetX()
	{
		return x;
	}

	float Vector3::GetY()
	{
		return y;
	}

	float Vector3::GetZ()
	{
		return z;
	}

	void Vector3::SetX(float newX)
	{
		x = newX;
	}

	void Vector3::SetY(float newY)
	{
		y = newY;
	}

	void Vector3::SetZ(float newZ)
	{
		z = newZ;
	}

	Vector3 Vector3::operator=(Vector3& toCopy)
	{
		x = toCopy.x;
		y = toCopy.y;
		z = toCopy.z;

		return *this;
	}

	Vector3 Vector3::operator=(Vector3 toCopy)
	{
		x = toCopy.x;
		y = toCopy.y;
		z = toCopy.z;

		return *this;
	}

	Vector3 Vector3::operator*(Vector3& right)
	{
		x *= right.x;
		y *= right.y;
		z *= right.z;

		return *this;
	}

	bool Vector3::operator==(const Vector3& right)
	{
		return (x == right.x && y == right.y && z == right.z);
	}

	bool Vector3::operator!=(const Vector3& right)
	{
		return !(x == right.x && y == right.y && z == right.z);
	}
}