#pragma once
#include <imgui.h>


namespace FlatEngine
{
	class Vector2 : public ImVec2
	{
	public:
		Vector2();
		Vector2(float xyValue);
		Vector2(float x, float y);
		Vector2(const ImVec2& toCopy);
		Vector2(const Vector2& toCopy);
		~Vector2();

		float GetX();
		float GetY();
		void SetX(float newX);
		void SetY(float newY);
		void _xy(float newX, float newY);
		Vector2 operator=(ImVec2& toCopy);
		Vector2 operator*(ImVec2& right);
		Vector2 operator*(Vector2& right);
		Vector2 operator+(const Vector2& right);
		Vector2 operator-(const Vector2& right);
		Vector2 NormalizeCardinal();
		Vector2 Normalize();
		bool operator==(const Vector2& right);
		bool operator!=(const Vector2& right);
	};
}
