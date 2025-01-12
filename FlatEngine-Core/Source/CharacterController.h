#pragma once
#include "Component.h"
#include "Vector2.h"

#include <string>

namespace FlatEngine {
	class CharacterController : public Component
	{
	public:
		CharacterController(long myID, long parentID);
		~CharacterController();
		std::string GetData();

		void MoveToward(Vector2 direction);
		void SetMaxAcceleration(float speed);
		float GetMaxAcceleration();
		void SetMaxSpeed(float newMaxSpeed);
		float GetMaxSpeed();
		void SetAirControl(float newAirControl);
		float GetAirControl();
		void SetMoving(bool b_isMoving);
		bool IsMoving();

	private:
		float m_maxAcceleration;
		float m_maxSpeed;
		float m_airControl;
		float m_speedCorrection;
		bool m_b_isMoving;
	};
}


