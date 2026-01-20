#pragma once
#include "CoreMinimal.h"
#include "../Public/AbstractMuseumDataTable.h"


class FMuseumStorageManager {
public:
#if WITH_EDITOR
	static void BuildAMShow() ;
	static void BuildAMStorage() ;
	static void GetDataTable() ;
	static void CreateDataTable() ;
#endif
};
