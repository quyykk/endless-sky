/* LoadingPanel.cpp
Copyright (c) 2021 by Michael Zahniser

Endless Sky is free software: you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later version.

Endless Sky is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more details.
*/

#include "LoadingPanel.h"

#include "Angle.h"
#include "Audio.h"
#include "GameData.h"
#include "Interface.h"
#include "Information.h"
#include "MenuPanel.h"
#include "MaskManager.h"
#include "PlayerInfo.h"
#include "Point.h"
#include "PointerShader.h"
#include "Ship.h"
#include "SpriteSet.h"
#include "StarField.h"
#include "System.h"
#include "UI.h"

#include "gl_header.h"

using namespace std;



LoadingPanel::LoadingPanel(PlayerInfo &player, UI &gamePanels, bool &finishedLoading)
	: player(player), gamePanels(gamePanels), finishedLoading(finishedLoading)
{
	SetIsFullScreen(true);
}



void LoadingPanel::Step()
{
	progress = static_cast<int>(GameData::Progress() * 60.);

	// If the game isn't loaded yet, upload any images to the GPU.
	GameData::ProcessSprites();

	if(progress == 60)
	{
		// Now that we have finished loading all the basic sprites and sounds, we can look for invalid file paths,
		// e.g. due to capitalization errors or other typos.
		SpriteSet::CheckReferences();
		Audio::CheckReferences();
		// All sprites with collision masks should also have their 1x scaled versions, so create
		// any additional scaled masks from the default one.
		GameData::GetMaskManager().ScaleMasks();

		GameData::FinishLoading();

		player.LoadRecent();

		GetUI()->Pop(this);
		GetUI()->Push(new MenuPanel(player, gamePanels));
		finishedLoading = true;
	}
}



void LoadingPanel::Draw()
{
	glClear(GL_COLOR_BUFFER_BIT);
	GameData::Background().Draw(Point(), Point());

	GameData::Interfaces().Get("menu background")->Draw(Information(), this);

	// Draw the loading circle.
	Angle da(6.);
	Angle a(0.);
	PointerShader::Bind();
	for(int i = 0; i < progress; ++i)
	{
		PointerShader::Add(Point(), a.Unit(), 8.f, 20.f, 140.f, Color(.5f, 0.f));
		a += da;
	}
	PointerShader::Unbind();
}
