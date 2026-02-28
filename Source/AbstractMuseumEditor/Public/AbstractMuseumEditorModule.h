#pragma once

#include "Modules/ModuleManager.h"

class FAbstractMuseumEditorModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
private:
	void RegisterCustomizations();
	void UnregisterCustomizations();
};