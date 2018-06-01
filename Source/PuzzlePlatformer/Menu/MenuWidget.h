// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MenuInterface.h"
#include "MenuWidget.generated.h"

/**
 * 
 */
UCLASS()
class PUZZLEPLATFORMER_API UMenuWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
    void Setup();
    void Teardown();

    void SetMenuInterface(IMenuInterface* pMenuInterface);
	
protected:
    IMenuInterface * MenuInterface;
};
