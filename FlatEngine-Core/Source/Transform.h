#pragma once
#include "Component.h"
#include "Vector2.h"
#include "Vector3.h"

#include <string>
#include "json.hpp"

using json = nlohmann::json;
using namespace nlohmann::literals;


namespace FlatEngine
{
	class Transform : public Component
	{
	public:
		Transform(long myID = -1, long parentID = -1);
		~Transform();
		std::string GetData();

		void SetInitialPosition(Vector2 initialPos);
		void SetOrigin(Vector2 newOrigin);
		Vector2 GetOrigin();
		void UpdateOrigin(Vector2 newOrigin);
		void UpdateChildOrigins(Vector2 newOrigin);
		Vector2 GetTruePosition();
		void SetPosition(Vector2 position);
		void SetBaseScale(Vector2 scale);
		void UpdateChildBaseScale(Vector2 scale);
		Vector2 GetBaseScale();
		Vector2 GetTotalScale();
		void SetScale(Vector2 scale);
		void SetRotation(float rotation);
		Vector2 GetPosition();
		Vector3 GetPosition3D();
		Vector2 GetScale();
		float GetRotation();
		void LookAt(Vector2 lookAt);

	private:
		Vector2 m_origin;
		Vector2 m_position;
		Vector3 m_position3D;
		Vector2 m_baseScale;
		Vector2 m_scale;
		float m_baseRotation;
		float m_rotation;
	};
}
