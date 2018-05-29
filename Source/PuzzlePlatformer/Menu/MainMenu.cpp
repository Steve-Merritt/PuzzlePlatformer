// Fill out your copyright notice in the Description page of Project Settings.

#include "MainMenu.h"
#include "UObject/ConstructorHelpers.h"
#include "Components/Button.h"
#include "Components/WidgetSwitcher.h"
#include "Components/EditableTextBox.h"
#include "Components/TextBlock.h"

#include "ServerRow.h"

UMainMenu::UMainMenu(const FObjectInitializer & ObjectInitializer)
	: Super(ObjectInitializer)
{
	ConstructorHelpers::FClassFinder<UUserWidget> ServerRowBPClass(TEXT("/Game/Menu/WBP_ServerRow"));
	if(!ensure(ServerRowBPClass.Class != nullptr)) return;

	ServerRowClass = ServerRowBPClass.Class;
}

bool UMainMenu::Initialize()
{
	if(!Super::Initialize()) return false;

	if(!ensure(HostButton != nullptr)) return false;
	HostButton->OnClicked.AddDynamic(this, &UMainMenu::OpenHostMenu);

	if(!ensure(JoinButton != nullptr)) return false;
	JoinButton->OnClicked.AddDynamic(this, &UMainMenu::OpenJoinMenu);

	if(!ensure(HostMenuHostButton != nullptr)) return false;
	HostMenuHostButton->OnClicked.AddDynamic(this, &UMainMenu::HostServer);

	if(!ensure(JoinMenuJoinButton != nullptr)) return false;
	JoinMenuJoinButton->OnClicked.AddDynamic(this, &UMainMenu::JoinServer);

	if(!ensure(JoinMenuBackButton != nullptr)) return false;
	JoinMenuBackButton->OnClicked.AddDynamic(this, &UMainMenu::OpenMainMenu);

	if(!ensure(HostMenuBackButton != nullptr)) return false;
	HostMenuBackButton->OnClicked.AddDynamic(this, &UMainMenu::OpenMainMenu);

	return true;
}

void UMainMenu::SetMenuInterface(IMenuInterface* pMenuInterface)
{
	MenuInterface = pMenuInterface;
}

void UMainMenu::Setup()
{
	this->AddToViewport();

	UWorld* pWorld = GetWorld();
	if(!ensure(pWorld != nullptr)) return;
	
	APlayerController* playerController = pWorld->GetFirstPlayerController();
	if(!ensure(playerController != nullptr)) return;

	FInputModeUIOnly inputMode;
	inputMode.SetWidgetToFocus(this->TakeWidget());
	inputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	playerController->SetInputMode(inputMode);

	playerController->bShowMouseCursor = true;
}

void UMainMenu::Teardown()
{
	this->RemoveFromViewport();
	
	UWorld* pWorld = GetWorld();
	if(!ensure(pWorld != nullptr)) return;

	APlayerController* playerController = pWorld->GetFirstPlayerController();
	if(!ensure(playerController != nullptr)) return;

	FInputModeGameOnly inputMode;
	playerController->SetInputMode(inputMode);

	playerController->bShowMouseCursor = false;
}

void UMainMenu::OpenHostMenu()
{
	if(!ensure(HostMenu != nullptr)) return;

	MenuSwitcher->SetActiveWidget(HostMenu);
}

void UMainMenu::HostServer()
{
	if(MenuInterface != nullptr)
	{
		FString ServerName = ServerHostName->Text.ToString();
		MenuInterface->Host(ServerName);
	}
}

void UMainMenu::OpenJoinMenu()
{
	if(!ensure(MenuSwitcher != nullptr)) return;
	if(!ensure(JoinMenu != nullptr)) return;

	MenuSwitcher->SetActiveWidget(JoinMenu);

	MenuInterface->RefreshServerList();
}

void UMainMenu::OpenMainMenu()
{
	if(!ensure(MenuSwitcher != nullptr)) return;
	if(!ensure(MainMenu != nullptr)) return;

	MenuSwitcher->SetActiveWidget(MainMenu);
}

void UMainMenu::JoinServer()
{
	if(SelectedIndex.IsSet() && MenuInterface != nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("Selected index %d"), SelectedIndex.GetValue())
		MenuInterface->Join(SelectedIndex.GetValue());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Selected index is not set."))
	}
}

void UMainMenu::SetServerList(const TArray<FServerData> &Servers)
{
	UWorld* World = this->GetWorld();
	if(!ensure(World != nullptr)) return;

	ServerList->ClearChildren();

	uint32 Index = 0;
	for(const auto& Server : Servers)
	{
		UServerRow* Row = CreateWidget<UServerRow>(World, ServerRowClass);
		if(!ensure(Row != nullptr)) return;

		FString PlayerCount = FString::Printf(TEXT("%d/%d"), Server.CurrentPlayers, Server.MaxPlayers);

		Row->ServerName->SetText(FText::FromString(Server.Name));
		Row->HostUsername->SetText(FText::FromString(Server.HostUsername));
		Row->PlayerCount->SetText(FText::FromString(PlayerCount));
		Row->Setup(this, Index);
		++Index;

		ServerList->AddChild(Row);
	}	
}

void UMainMenu::SelectIndex(uint32 Index)
{
	SelectedIndex = Index;
	UpdateChildren();
}

void UMainMenu::UpdateChildren()
{
	for(int32 i = 0; i < ServerList->GetChildrenCount(); ++i)
	{
		UServerRow* Row = Cast<UServerRow>(ServerList->GetChildAt(i));
		if(Row != nullptr)
		{
			Row->Selected = (SelectedIndex.IsSet() && SelectedIndex.GetValue() == i);
		}
	}
}
