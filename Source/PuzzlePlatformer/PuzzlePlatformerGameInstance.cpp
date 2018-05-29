// Fill out your copyright notice in the Description page of Project Settings.

#include "PuzzlePlatformerGameInstance.h"
#include "Engine/Engine.h"
#include "UObject/ConstructorHelpers.h"
#include "Blueprint/UserWidget.h"
#include "Menu/MainMenu.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"

static const FName SERVER_NAME_SETTINGS_KEY = TEXT("ServerName");
static const FName HOST_MATCH_ID = TEXT("HostMatchId");

#define SEARCH_VERSION_NUMBER FName(TEXT("VERSIONNUMBER"))

UPuzzlePlatformerGameInstance::UPuzzlePlatformerGameInstance(const FObjectInitializer & ObjectInitializer)
	: Super(ObjectInitializer)
{
	ConstructorHelpers::FClassFinder<UUserWidget> MenuBPClass(TEXT("/Game/Menu/WBP_MainMenu"));
	if(!ensure(MenuBPClass.Class != nullptr)) return;

	MenuClass = MenuBPClass.Class;

	// Delegate handles
	OnCreateSessionCompleteDelegate = FOnCreateSessionCompleteDelegate::CreateUObject(this, &UPuzzlePlatformerGameInstance::OnCreateSessionComplete);
	OnDestroySessionCompleteDelegate = FOnDestroySessionCompleteDelegate::CreateUObject(this, &UPuzzlePlatformerGameInstance::OnDestroySessionComplete);
	OnFindSessionsCompleteDelegate = FOnFindSessionsCompleteDelegate::CreateUObject(this, &UPuzzlePlatformerGameInstance::OnFindSessionsComplete);
	OnJoinSessionCompleteDelegate = FOnJoinSessionCompleteDelegate::CreateUObject(this, &UPuzzlePlatformerGameInstance::OnJoinSessionComplete);
	OnStartSessionCompleteDelegate = FOnStartSessionCompleteDelegate::CreateUObject(this, &UPuzzlePlatformerGameInstance::OnStartOnlineGameComplete);

}

void UPuzzlePlatformerGameInstance::Init()
{
	// Get a pointer to the OSS interface.
	IOnlineSubsystem* pOSS = IOnlineSubsystem::Get();
	if(pOSS)
	{
		SessionInterface = pOSS->GetSessionInterface();
		if(SessionInterface.IsValid())
		{
			UE_LOG(LogTemp, Warning, TEXT("Found subsystem %s"), *pOSS->GetSubsystemName().ToString());

			// Initialize session delegates
			OnCreateSessionCompleteDelegateHandle = SessionInterface->AddOnCreateSessionCompleteDelegate_Handle(OnCreateSessionCompleteDelegate);
			OnDestroySessionCompleteDelegateHandle = SessionInterface->AddOnDestroySessionCompleteDelegate_Handle(OnDestroySessionCompleteDelegate);
			SessionInterface->AddOnFindSessionsCompleteDelegate_Handle(OnFindSessionsCompleteDelegate);
			SessionInterface->AddOnJoinSessionCompleteDelegate_Handle(OnJoinSessionCompleteDelegate);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Failed to get a pointer to IOnlineSession."));
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("No OSS found."));
	}
}

void UPuzzlePlatformerGameInstance::LoadMenuWidget()
{
	if(!ensure(MenuClass != nullptr)) return;

	MainMenu = CreateWidget<UMainMenu>(this, MenuClass);
	if(!ensure(MainMenu != nullptr)) return;

	MainMenu->Setup();
	MainMenu->SetMenuInterface(this);
}

void UPuzzlePlatformerGameInstance::Host(const FString& ServerName)
{
	DesiredServerName = ServerName;

	// Fire up the Online Sub-system for network play.
	if(SessionInterface.IsValid())
	{
		auto existingSession = SessionInterface->GetNamedSession(NAME_GameSession);
		if(existingSession != nullptr)
		{
			SessionInterface->DestroySession(NAME_GameSession);
		}
		else
		{
			CreateSession();
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Host Failed: No session interface."));
	}

}

void UPuzzlePlatformerGameInstance::Join(uint32 Index)
{
	if(!SessionInterface.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("Invalid Session Interface pointer."));
		return;
	}

	if(!SessionSearch.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("Invalid Session Search pointer."));
		return;
	}

	if(MainMenu != nullptr)
	{
		MainMenu->Teardown();
	}

	UE_LOG(LogTemp, Warning, TEXT("Joining Session %s"), *SessionSearch->SearchResults[Index].GetSessionIdStr());
	SessionInterface->JoinSession(0, NAME_GameSession, SessionSearch->SearchResults[Index]);
}

void UPuzzlePlatformerGameInstance::RefreshServerList()
{
	if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 8, FColor::Green, TEXT("Refreshing server list..."));
	UE_LOG(LogTemp, Warning, TEXT("Refreshing server list..."));

	// Search for existing sessions
	SessionSearch = MakeShareable(new FOnlineSessionSearch());
	if(SessionSearch.IsValid())
	{
		SessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);
		
		IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
		if(OnlineSub && OnlineSub->GetSubsystemName() == "NULL")
		{
			SessionSearch->bIsLanQuery = true;
		}
		else
		{
			SessionSearch->bIsLanQuery = false;
		}

		SessionSearch->MaxSearchResults = 20;
		SessionSearch->PingBucketSize = 50;

		UE_LOG(LogTemp, Warning, TEXT("Looking for online sessions..."));
		SessionInterface->FindSessions(0, SessionSearch.ToSharedRef());
	}
}

void UPuzzlePlatformerGameInstance::CreateSession()
{
	if(SessionInterface.IsValid())
	{
		SessionSettings = MakeShareable(new FOnlineSessionSettings());
		if(!SessionSettings.IsValid())
		{
			UE_LOG(LogTemp, Warning, TEXT("Failed to create host settings."));
			return;
		}

		IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
		if(OnlineSub && OnlineSub->GetSubsystemName() == "NULL")
		{
			SessionSettings->bIsLANMatch = true;
		}
		else
		{
			SessionSettings->bIsLANMatch = false;
		}
		
		UE_LOG(LogTemp, Warning, TEXT("SessionSettings::bIsLANMatch: %d"), SessionSettings->bIsLANMatch ? 1 : 0);

		SessionSettings->NumPublicConnections = 3;
		SessionSettings->NumPrivateConnections = 0;
		SessionSettings->bAllowInvites = true;
		SessionSettings->bAllowJoinInProgress = true;
		SessionSettings->bShouldAdvertise = true;
		SessionSettings->bUsesPresence = true;
		SessionSettings->bAllowJoinViaPresence = true;
		SessionSettings->bAllowJoinViaPresenceFriendsOnly = false;
		
		FString MapName("Lobby");
		SessionSettings->Set(SETTING_MAPNAME, MapName, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
		SessionSettings->Set(SERVER_NAME_SETTINGS_KEY, DesiredServerName, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);

		//FString GameType("/Script/PuzzlePlatformer.PuzzlePlatformerGameMode");
		//SessionSettings->Set(SETTING_GAMEMODE, GameType, EOnlineDataAdvertisementType::ViaOnlineService);
		//SessionSettings->Set(SEARCH_VERSION_NUMBER, GetGameVersionNumber(), EOnlineDataAdvertisementType::ViaOnlineService);

		//DumpSessionSettings(SessionSettings.Get());
		SessionInterface->CreateSession(0, NAME_GameSession, *SessionSettings);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to create a new session."));
	}
}

// OSS Callbacks

void UPuzzlePlatformerGameInstance::OnCreateSessionComplete(FName InSessionName, bool bWasSuccessful)
{
	if(!bWasSuccessful)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to create a new game session."));
		return;
	}

	if(MainMenu != nullptr)
	{
		MainMenu->Teardown();
	}

	UEngine* pEngine = GetEngine();
	if(!ensure(pEngine != nullptr)) return;

	pEngine->AddOnScreenDebugMessage(-1, 300, FColor::Green, TEXT("Hosting"));

	// Get the OnlineSubsystem so we can get the Session Interface
	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
	if(OnlineSub)
	{
		// Get the Session Interface to call the StartSession function
		IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();

		if(Sessions.IsValid())
		{
			// Clear the SessionComplete delegate handle, since we finished this call
			Sessions->ClearOnCreateSessionCompleteDelegate_Handle(OnCreateSessionCompleteDelegateHandle);
			if(bWasSuccessful)
			{
				// Set the StartSession delegate handle
				OnStartSessionCompleteDelegateHandle = Sessions->AddOnStartSessionCompleteDelegate_Handle(OnStartSessionCompleteDelegate);

				// Our StartSessionComplete delegate should get called after this
				UE_LOG(LogTemp, Warning, TEXT("Starting new game session: %s"), *InSessionName.ToString());
				Sessions->StartSession(InSessionName);
			}
		}
	}	
}

void UPuzzlePlatformerGameInstance::OnStartOnlineGameComplete(FName SessionName, bool bWasSuccessful)
{
	// Get the Online Subsystem so we can get the Session Interface
	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
	if(OnlineSub)
	{
		// Get the Session Interface to clear the Delegate
		IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();
		if(Sessions.IsValid())
		{
			// Clear the delegate, since we are done with this call
			Sessions->ClearOnStartSessionCompleteDelegate_Handle(OnStartSessionCompleteDelegateHandle);
		}
	}

	if(bWasSuccessful)
	{
		// get the map name from the session settings
		FString MapName;
		SessionSettings->Get(SETTING_MAPNAME, MapName);
		FString GameMode;
		SessionSettings->Get(SETTING_GAMEMODE, GameMode);
		int32 MaxPlayers = SessionSettings->NumPublicConnections + SessionSettings->NumPrivateConnections;
		// and open it
		//FString Parameters = FString::Printf(TEXT("/Game/ThirdPersonCPP/Maps/%s?listen?game=%s?MaxPlayers=%i"), *MapName, *GameMode, MaxPlayers);
		FString Parameters = FString::Printf(TEXT("/Game/Maps/%s?listen"), *MapName);

		// the bIsLanMatch parameter is needed so the online subsystem creates a standard listen socket instead of a steam socket
		if(SessionSettings->bIsLANMatch)
		{
			Parameters.Append("?bIsLanMatch=1");
		}

		UWorld* World = GetWorld();
		if(!ensure(World != nullptr)) return;

		UE_LOG(LogTemp, Warning, TEXT("ServerTravel with Parameters: %s"), *Parameters);
		World->ServerTravel(Parameters);
	}
}

void UPuzzlePlatformerGameInstance::OnDestroySessionComplete(FName InSessionName, bool bWasSuccessful)
{
	UE_LOG(LogTemp, Warning, TEXT("OnDestroySessionComplete %s bSuccess: %d"), *InSessionName.ToString(), bWasSuccessful);

	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
	if(OnlineSub)
	{
		IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();
		Sessions->ClearOnDestroySessionCompleteDelegate_Handle(OnDestroySessionCompleteDelegateHandle);
		SessionSettings = nullptr;
	}
}

void UPuzzlePlatformerGameInstance::OnFindSessionsComplete(bool bWasSuccessful)
{
	if (GEngine) GEngine->ClearOnScreenDebugMessages();
	UE_LOG(LogTemp, Warning, TEXT("OnFindSessionsComplete bSuccess: %d"), bWasSuccessful);

	if(bWasSuccessful && SessionInterface.IsValid())
	{
		//Sessions->ClearOnFindSessionsCompleteDelegate_Handle(OnFindSessionsCompleteDelegateHandle);

		UE_LOG(LogTemp, Warning, TEXT("Num Search Results: %d"), SessionSearch->SearchResults.Num());
		TArray<FServerData> ServerList;
		if(SessionSearch->SearchResults.Num() > 0)
		{
			for(const auto& SearchResult : SessionSearch->SearchResults)
			{
				FServerData Data;

				FString ServerName;
				if(SearchResult.Session.SessionSettings.Get(SERVER_NAME_SETTINGS_KEY, ServerName))
				{
					Data.Name = ServerName;
				}
				else
				{
					Data.Name = "Unknown";
				}
				
				Data.MaxPlayers = SearchResult.Session.SessionSettings.NumPublicConnections;
				Data.CurrentPlayers = Data.MaxPlayers - SearchResult.Session.NumOpenPublicConnections; // Note: Doesn't work with NULL OSS.
				Data.HostUsername = SearchResult.Session.OwningUserName;

				ServerList.Add(Data);
				UE_LOG(LogTemp, Warning, TEXT("    Result: %s %s"), *Data.Name, *Data.HostUsername);
			}
		}
		else
		{
			FServerData Data;
			Data.Name = "No servers found.";
			ServerList.Add(Data);
		}

        MainMenu->SetServerList(ServerList);
	}
}

void UPuzzlePlatformerGameInstance::OnJoinSessionComplete(FName InSessionName, EOnJoinSessionCompleteResult::Type Result)
{
	bool bWillTravel = false;

	UE_LOG(LogTemp, Warning, TEXT("OnJoinSessionComplete SessionName: %s"), *InSessionName.ToString());

	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
	IOnlineSessionPtr Session = NULL;
	if(OnlineSub)
	{
		Session = OnlineSub->GetSessionInterface();
		if(Session.IsValid())
		{
			FString ConnectInfo;
			Session->GetResolvedConnectString(InSessionName, ConnectInfo);

			UE_LOG(LogTemp, Warning, TEXT("ConnectString: %s"), *ConnectInfo);

			UEngine* pEngine = GetEngine();
			if(!ensure(pEngine != nullptr)) return;

			pEngine->AddOnScreenDebugMessage(-1, 300, FColor::Green, FString::Printf(TEXT("Joining %s"), *ConnectInfo));

			APlayerController* playerController = GetFirstLocalPlayerController();
			if(!ensure(playerController != nullptr)) return;

			playerController->ClientTravel(ConnectInfo, ETravelType::TRAVEL_Absolute);
		}	
	}
}
