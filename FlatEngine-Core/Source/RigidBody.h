#pragma once
#include "Component.h"
#include "CharacterController.h"
#include "Collider.h"
#include "Vector2.h"

#include <string>


namespace FlatEngine 
{
	class RigidBody : public Component
	{
		friend CharacterController;

	public:
		RigidBody(long myID = -1, long parentID = -1);
		~RigidBody();
		std::string GetData();

		void CalculatePhysics();
		void ApplyPhysics(float deltaTime);
		Vector2 AddVelocity(Vector2 vel);
		void ApplyGravity();
		void AddForce(Vector2 force, float multiplier = 1);
		void AddTorque(float torque, float multiplier = 1);
		void ApplyEquilibriumForce();
		void ApplyCollisionForces();
		void ApplyCollisionForce(Collider* collider, float halfWidth, float halfHeight);

		Vector2 GetNextPosition();

		void SetMass(float newMass);
		float GetMass();
		float GetI();
		void UpdateI();
		void SetTorquesAllowed(bool b_allowed);
		bool TorquesAllowed();
		void SetAngularDrag(float newAngularDrag);
		float GetAngularDrag();
		void SetGravity(float newGravity);
		float GetGravity();
		void SetFallingGravity(float newFallingGravity);
		float GetFallingGravity();
		Vector2 GetVelocity();
		Vector2 GetAcceleration();
		Vector2 GetPendingForces();
		void SetVelocity(Vector2 newVelocity);
		void SetPendingForces(Vector2 newPendingForces);
		void SetAngularVelocity(float angularVelocity);
		float GetAngularVelocity();
		float GetAngularAcceleration();
		float GetPendingTorques();
		void SetWindResistance(float newWindResistance);
		float GetWindResistance();
		float GetFriction();
		void SetFriction(float newFriction);
		void SetTerminalVelocity(float newTerminalVelocity);
		float GetTerminalVelocity();
		void SetEquilibriumForce(float newEquilibriumForce);
		float GetEquilibriumForce();
		void ApplyFriction();
		bool IsStatic();
		void SetIsStatic(bool b_static);
		void SetIsGrounded(bool b_grounded);
		bool IsGrounded();

	private:
		// Linear
		float m_mass;
		float m_1overMass;
		Vector2 m_pendingForces;
		Vector2 m_velocity;
		Vector2 m_acceleration;
		float m_friction;
		// Rotational
		float m_I;
		float m_1overI;
		float m_pendingTorques;
		float m_angularVelocity;
		float m_angularAcceleration;
		float m_angularDrag;
		bool m_b_allowTorques;

		float m_forceCorrection;
		bool m_b_isGrounded;
		bool m_b_isStatic;
		float m_windResistance;
		float m_equilibriumForce;
		float m_gravity;
		float m_fallingGravity;
		float m_terminalVelocity;
	};
}