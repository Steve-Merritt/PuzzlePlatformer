// Fill out your copyright notice in the Description page of Project Settings.

#include "LobbyGameMode.h"


void ALobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	NumberOfPlayers++;
	UE_LOG(LogTemp, Warning, TEXT("Number of players: %d"), NumberOfPlayers);

	if(NumberOfPlayers >= 3)
	{
		UWorld* World = GetWorld();
		if(!ensure(World != nullptr)) return;

		bUseSeamlessTravel = true;
		World->ServerTravel("/Game/Maps/Game?listen");
	}
}

void ALobbyGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);

	NumberOfPlayers--;
}
