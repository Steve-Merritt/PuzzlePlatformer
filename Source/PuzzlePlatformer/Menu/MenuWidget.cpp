// Fill out your copyright notice in the Description page of Project Settings.

#include "MenuWidget.h"

void UMenuWidget::Setup()
{
    this->AddToViewport();

    UWorld* pWorld = GetWorld();
    if (!ensure(pWorld != nullptr)) return;

    APlayerController* playerController = pWorld->GetFirstPlayerController();
    if (!ensure(playerController != nullptr)) return;

    FInputModeUIOnly inputMode;
    inputMode.SetWidgetToFocus(this->TakeWidget());
    inputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
    playerController->SetInputMode(inputMode);

    playerController->bShowMouseCursor = true;
}

void UMenuWidget::Teardown()
{
    this->RemoveFromViewport();

    UWorld* pWorld = GetWorld();
    if (!ensure(pWorld != nullptr)) return;

    APlayerController* playerController = pWorld->GetFirstPlayerController();
    if (!ensure(playerController != nullptr)) return;

    FInputModeGameOnly inputMode;
    playerController->SetInputMode(inputMode);

    playerController->bShowMouseCursor = false;
}

void UMenuWidget::SetMenuInterface(IMenuInterface* pMenuInterface)
{
    MenuInterface = pMenuInterface;
}
