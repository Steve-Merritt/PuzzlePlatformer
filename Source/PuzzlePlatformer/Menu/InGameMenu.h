// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Menu/MenuWidget.h"
#include "InGameMenu.generated.h"

/**
 * 
 */
UCLASS()
class PUZZLEPLATFORMER_API UInGameMenu : public UMenuWidget
{
	GENERATED_BODY()
	
public:
    UFUNCTION(BlueprintCallable)
    void CancelPressed();

    UFUNCTION(BlueprintCallable)
    void QuitPressed();
	
};
