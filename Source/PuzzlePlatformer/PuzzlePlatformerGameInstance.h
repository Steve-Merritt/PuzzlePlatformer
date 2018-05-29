// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Menu/MenuInterface.h"
#include "OnlineSessionInterface.h"

#include "PuzzlePlatformerGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class PUZZLEPLATFORMER_API UPuzzlePlatformerGameInstance : public UGameInstance, public IMenuInterface
{
	GENERATED_BODY()
	
public:
	UPuzzlePlatformerGameInstance(const FObjectInitializer & ObjectInitializer);

	virtual void Init();
	
	UFUNCTION(BlueprintCallable)
	void LoadMenuWidget();

	
	// MenuInterface Impl

	UFUNCTION(Exec)
	void Host(const FString& ServerName);

	UFUNCTION(Exec)
	void Join(uint32 Index);

	UFUNCTION(Exec)
	void RefreshServerList();

protected:
	/** Delegate for creating a new session */
	FOnCreateSessionCompleteDelegate OnCreateSessionCompleteDelegate;
	/** Delegate after starting a session */
	FOnStartSessionCompleteDelegate OnStartSessionCompleteDelegate;
	/** Delegate for destroying a session */
	FOnDestroySessionCompleteDelegate OnDestroySessionCompleteDelegate;
	/** Delegate for searching for sessions */
	FOnFindSessionsCompleteDelegate OnFindSessionsCompleteDelegate;
	/** Delegate after joining a session */
	FOnJoinSessionCompleteDelegate OnJoinSessionCompleteDelegate;

	/** Handles to various registered delegates */
	FDelegateHandle OnStartSessionCompleteDelegateHandle;
	FDelegateHandle OnCreateSessionCompleteDelegateHandle;

	/** Current host settings */
	TSharedPtr<class FOnlineSessionSettings> SessionSettings;
	/** Current search settings */
	TSharedPtr<class FOnlineSessionSearch> SessionSearch;

	/**
	* Delegate fired when a session create request has completed
	*
	* @param SessionName the name of the session this callback is for
	* @param bWasSuccessful true if the async action completed without error, false if there was an error
	*/
	void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);

	/**
	* Delegate fired when a session start request has completed
	*
	* @param SessionName the name of the session this callback is for
	* @param bWasSuccessful true if the async action completed without error, false if there was an error
	*/
	void OnStartOnlineGameComplete(FName SessionName, bool bWasSuccessful);

	/**
	* Delegate fired when a destroying an online session has completed
	*
	* @param SessionName the name of the session this callback is for
	* @param bWasSuccessful true if the async action completed without error, false if there was an error
	*/
	void OnDestroySessionComplete(FName SessionName, bool bWasSuccessful);

	/**
	* Delegate fired when a session search query has completed
	*
	* @param bWasSuccessful true if the async action completed without error, false if there was an error
	*/
	void OnFindSessionsComplete(bool bWasSuccessful);

	/**
	* Delegate fired when a session join request has completed
	*
	* @param SessionName the name of the session this callback is for
	* @param bWasSuccessful true if the async action completed without error, false if there was an error
	*/
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);

private:
	void CreateSession();

	TSubclassOf<class UUserWidget> MenuClass;
	class UMainMenu* MainMenu;

	IOnlineSessionPtr SessionInterface;

	FDelegateHandle OnDestroySessionCompleteDelegateHandle;

	FString DesiredServerName;
};
