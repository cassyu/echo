#pragma once

#include "engine/core/main/module.h"
#include "physx_debug_draw.h"
#include "physx_base.h"

namespace Echo
{
	class PhysxModule : public Module 
	{
		ECHO_SINGLETON_CLASS(PhysxModule, Module)

	public:
		PhysxModule();
        virtual ~PhysxModule();

		// instance
		static PhysxModule* instance();

		// resister all types of the module
		virtual void registerTypes() override;

		// update physx world
		virtual  void update(float elapsedTime) override;

		// get pxPhysics
		physx::PxPhysics* getPxPhysics() { return m_pxPhysics; }

		// get scene
		physx::PxScene* getPxScene() { return m_pxScene; }
        
    public:
        // debug draw
        const StringOption& getDebugDrawOption() const { return m_drawDebugOption; }
        void setDebugDrawOption(const StringOption& option);

		// gravity
		const Vector3& getGravity() const { return m_gravity; }
		void setGravity(const Vector3& gravity);

		// shift origin
		const Vector3& getShift() const { return m_shift; }
		void setShift(const Vector3& shift);

	private:
		// initialize
		bool initPhysx();
        
    private:
        StringOption					m_drawDebugOption = StringOption("Editor", { "None","Editor","Game","All" });
		physx::PxAllocatorCallback*		m_pxAllocatorCb = nullptr;
		physx::PxErrorCallback*			m_pxErrorCb = nullptr;
		physx::PxFoundation*			m_pxFoundation = nullptr;
		physx::PxPhysics*				m_pxPhysics = nullptr;
		physx::PxDefaultCpuDispatcher*	m_pxCPUDispatcher = nullptr;
		Vector3							m_gravity = Vector3(0.f, -9.8f, 0.f);
		Vector3							m_shift = Vector3::ZERO;
		physx::PxScene*					m_pxScene = nullptr;
		float							m_stepLength = 0.025f;
		float							m_accumulator = 0.f;
		PhysxDebugDraw*					m_debugDraw = nullptr;
	};
}
