#pragma once


namespace FlatEngine
{
	class Vector3
	{
	public:
		Vector3();
		Vector3(float xyzwValue);
		Vector3(float x, float y, float z);
		Vector3(const Vector3& toCopy);
		~Vector3();

		void _xyz(float newX, float newY, float newZ);
		float GetX();
		float GetY();
		float GetZ();		
		void SetX(float newX);
		void SetY(float newY);
		void SetZ(float newZ);		
		Vector3 operator=(Vector3& toCopy);
		Vector3 operator=(Vector3 toCopy);
		Vector3 operator*(Vector3& right);
		bool operator==(const Vector3& right);
		bool operator!=(const Vector3& right);

		float x;
		float y;
		float z;

	private:
	};
}

