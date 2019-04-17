// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once 

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "CaptureTheFlagHUD.generated.h"

UCLASS()
class ACaptureTheFlagHUD : public AHUD
{
	GENERATED_BODY()

public:
	ACaptureTheFlagHUD();

	/** Primary draw call for the HUD */
	virtual void DrawHUD() override;

	virtual void BeginPlay() override;

private:
	/** Crosshair asset pointer */
	class UTexture2D* CrosshairTex;
	UPROPERTY(EditAnywhere, Category = "Capture The Flag")
	TSubclassOf<class UUserWidget> HUDWidgetClass;

	UPROPERTY(EditAnywhere, Category = "Capture The Flag")
	class UUserWidget* CurrentWidget;
};

