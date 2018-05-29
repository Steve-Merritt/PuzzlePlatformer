// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class PuzzlePlatformerTarget : TargetRules
{
	public PuzzlePlatformerTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		bUsesSteam = true;
		ExtraModuleNames.Add("PuzzlePlatformer");
	}
}
