// Fill out your copyright notice in the Description page of Project Settings.

#include "InGameMenu.h"


void UInGameMenu::CancelPressed()
{
    Teardown();
}

void UInGameMenu::QuitPressed()
{
    if (MenuInterface != nullptr)
    {
        Teardown();
        MenuInterface->LoadMainMenu();
    }
}

