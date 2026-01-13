#include "Misc/AutomationTest.h"
#include "UObject/UObjectIterator.h"
#include "UObject/UObjectGlobals.h"
#include "Materials/MaterialInstanceDynamic.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST (FMIDTransientTest,
	"AbstractMuseum.Materials.MIDMustBeTransient",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMIDTransientTest::RunTest(const FString& Parameters)
{
	for (TObjectIterator <UMaterialInstanceDynamic> It; It; ++It)
	{
		UMaterialInstanceDynamic* MID = *It;
		if (!IsValid(MID))	continue;

		if (MID->IsUnreachable()) continue;

		if (MID->HasAnyFlags(RF_BeginDestroyed)) continue;

		UObject* Outer = MID->GetOuter();
		if (!Outer)
			continue;

		if (!Outer->HasAnyFlags(RF_ClassDefaultObject))
			continue;

		if (!MID->HasAnyFlags(RF_Transient))
		{
			AddError(FString::Printf(
				TEXT("MID '%s' is owned by CDO '%s'"),
				*MID->GetFullName(),
				*Outer->GetFullName()));
		}
	}
	return true;
}