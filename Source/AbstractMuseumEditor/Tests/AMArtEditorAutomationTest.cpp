#include "Misc/AutomationTest.h"
#include "Editor.h"
#include "Engine/World.h"
#include "AssetToolsModule.h"
#include "Tests/AutomationCommon.h"
#include "AbstractMuseumArt.h"
#include "FileHelpers.h"
#include "Editor/UnrealEd/Public/Kismet2/KismetEditorUtilities.h"
#include "IAssetTools.h"
#include "Factories/BlueprintFactory.h"
#include "Tests/AutomationEditorCommon.h"
#include "Engine/StreamableManager.h"
#include "Engine/AssetManager.h"


BEGIN_DEFINE_SPEC(
	FAMArtEditorSpec,
	"AbstractMuseum.Editor",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
	UWorld* World = nullptr;
	bool bMapLoaded = false;
	FString TestFolder = TEXT("/AbstractMuseum/TestMap");
END_DEFINE_SPEC(FAMArtEditorSpec)

void FAMArtEditorSpec::Define() {
	BeforeEach([this]()
		{
			if(!bMapLoaded)
			{
			const FString MapPath =
				TEXT("/AbstractMuseum/TestMap/NewMap");

			AutomationOpenMap(MapPath);
			bMapLoaded = true;
			}
		World = GEditor
				? GEditor->GetEditorWorldContext().World()
				: nullptr;

		TestNotNull(TEXT("Editor world exists"), World);
	});

	AfterEach([this]()
		{World = nullptr; }
	);

	Describe("AMArt", [this]()
		{
			It("Creates BP, assigns struct and file path, then spawns", [this]()
				{
					// --- Load ArtMaterialStruct asset ---
					const FString StructPath =
						TestFolder + TEXT("/test_amstruct.test_amstruct");

					UArtMaterialStruct* ArtStruct =
						LoadObject<UArtMaterialStruct>(nullptr, *StructPath);

					TestNotNull(TEXT("ArtMaterialStruct loaded"), ArtStruct);

					// --- Create Blueprint asset ---
					FAssetToolsModule& AssetToolsModule =
						FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools");

					UBlueprintFactory* Factory = NewObject<UBlueprintFactory>();
					Factory->ParentClass = AAbstractMuseumArt::StaticClass();

					UObject* NewAsset = AssetToolsModule.Get().CreateAsset(
						TEXT("BP_TestArt"),
						TestFolder,
						UBlueprint::StaticClass(),
						Factory
					);

					TestNotNull(TEXT("Blueprint created"), NewAsset);

					UBlueprint* BP = Cast<UBlueprint>(NewAsset);
					TestNotNull(TEXT("Blueprint cast valid"), BP);

					// --- Assign defaults on CDO ---
					AAbstractMuseumArt* CDO =
						Cast<AAbstractMuseumArt>(BP->GeneratedClass->GetDefaultObject());

					TestNotNull(TEXT("CDO exists"), CDO);

					CDO->ArtMaterialStruct = ArtStruct;
					const FString AbsoluteImagePath =
						FPaths::ConvertRelativePathToFull(
							FPaths::ProjectPluginsDir() +
							TEXT("AbstractMuseum/Content/TestMap/test_img.jpg")
						);

					CDO->LocalFilePath = AbsoluteImagePath;
					

					// Force BP to recompile with new defaults
					FKismetEditorUtilities::CompileBlueprint(BP);


					FVector SpawnLocation(50.f, 50.f, 170.f);
					FRotator SpawnRotation = FRotator::ZeroRotator;

					AAbstractMuseumArt* Art =
						World->SpawnActor<AAbstractMuseumArt>(
							BP->GeneratedClass,
							SpawnLocation,
							SpawnRotation
						);

					TestNotNull(TEXT("AMArt spawned"), Art);

					// --- Validate data on instance ---
					TestEqual(
						TEXT("ArtMaterialStruct assigned"),
						Art->ArtMaterialStruct,
						ArtStruct
					);

					TestTrue(
						TEXT("LocalFilePath assigned"),
						Art->LocalFilePath.Contains(TEXT("test_img.jpg"))
					
					);
				});
		});
}