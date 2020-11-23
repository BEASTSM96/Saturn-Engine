#pragma once

#include "EventListener.h"
#include <reactphysics3d/reactphysics3d.h>

namespace Saturn {

	class Scene;

	STRUCT(EngineSettings)
	public:
		long double elapsedTime;                        // Elapsed time (in seconds)
		float timeStep;                                 // Current time step (in seconds)
		unsigned int nbVelocitySolverIterations;        // Nb of velocity solver iterations
		unsigned int nbPositionSolverIterations;        // Nb of position solver iterations
		bool isSleepingEnabled;                         // True if sleeping technique is enabled
		float timeBeforeSleep;                          // Time of inactivity before a body sleep
		float sleepLinearVelocity;                      // Sleep linear velocity
		float sleepAngularVelocity;                     // Sleep angular velocity
		bool isGravityEnabled;                          // True if gravity is enabled

		EngineSettings() : elapsedTime(0.0f), timeStep(0.0f) {}

		static EngineSettings DefaultSettings() {

			EngineSettings defaultSettings;

			reactphysics3d::PhysicsWorld::WorldSettings worldSettings;
			defaultSettings.timeStep = 1.0f / 60.0f;
			defaultSettings.nbVelocitySolverIterations = worldSettings.defaultVelocitySolverNbIterations;
			defaultSettings.nbPositionSolverIterations = worldSettings.defaultPositionSolverNbIterations;
			defaultSettings.isSleepingEnabled = worldSettings.isSleepingEnabled;
			defaultSettings.timeBeforeSleep = worldSettings.defaultTimeBeforeSleep;
			defaultSettings.sleepLinearVelocity = worldSettings.defaultSleepLinearVelocity;
			defaultSettings.sleepAngularVelocity = worldSettings.defaultSleepAngularVelocity;
			defaultSettings.isGravityEnabled = true;

			return defaultSettings;
		}
		STRUCT_END();


	class PhysicsScene : public RefCounted {
	public:
		PhysicsScene(Scene* scene);
		~PhysicsScene();

		void Update(float delta);

		inline float GetUpdateTime() { return m_updateTime; }

		void ContactStay(reactphysics3d::CollisionBody* body, reactphysics3d::CollisionBody* other);
		void ContactEnter(reactphysics3d::CollisionBody* body, reactphysics3d::CollisionBody* other);
		void ContactExit(reactphysics3d::CollisionBody* body, reactphysics3d::CollisionBody* other);
	private:
		reactphysics3d::PhysicsCommon m_common;
		reactphysics3d::PhysicsWorld* m_world;

		float m_accumulator{ 0.0f };

		friend class Rigidbody;

		EventListener m_eventListener;

		Scene* m_scene;

		bool dd = true;

		float m_updateTime{ 0.0f };
		float m_oldTimeSinceStart{ 0.0f };

		glm::mat4 Mat4FromReactMat4(const reactphysics3d::Transform& matrix);
		glm::vec3 Vec3FromReactQuaternion(const reactphysics3d::Quaternion& matrix);
		glm::vec3 Vec3FromReactVec3(const reactphysics3d::Vector3& matrix);
	private:
		friend class Scene;

	};
}
