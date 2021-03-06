#pragma once

#include "engine/core/main/module.h"

namespace Echo
{
	class ModelModule : public Module
	{
		ECHO_SINGLETON_CLASS(ModelModule, Module)

	public:
		ModelModule();

		// instance
		static ModelModule* instance();

		// register all types of the module
		virtual void registerTypes() override;
	};
}
