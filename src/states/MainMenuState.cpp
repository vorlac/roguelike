#include "MainMenuState.h"
#include "TestState.h"
#include "../StateMachine.h"
#include "../UI/Window.h"
#include "../Debug.h"


MainMenuState::MainMenuState(Application *app) : 
	State(app), mainMenuWindow(100, 100, 300, 200, "sumbitch")
{
	name = "MainMenuState";
	keyboardContextID = "main_menu";
	font = LoadFontEx("data/fonts/Px437_IBM_EGA_9x14.ttf", 14, 0, 250);
	
	mainMenuWindow.centered = true;
	mainMenu.AddSelection("play");
	mainMenu.AddSelection("settings");
	mainMenu.AddSelection("quit");

}

void MainMenuState::OnLoad()
{
	
}

void MainMenuState::OnUpdate(double delta)
{

}

void MainMenuState::OnDraw()
{
	DrawWindow(mainMenuWindow, application->GetSettings());
	DrawMenu(mainMenu, &mainMenuUIE);
	Color white = { 255, 255, 255, 255 };
	DrawTexture(mainMenuUIE.GetTexture().texture, 100, 100, white);
}

void MainMenuState::OnDebugDraw()
{
	State::OnDebugDraw();
}

void MainMenuState::OnEvent(Event event)
{
	if (event.type == KeyboardEvent)
	{
		if (event.gameEvent == "debug_toggle")
			ToggleDebug();
		else if (event.gameEvent == "menu_up")
			mainMenu.Prev();
		else if (event.gameEvent == "menu_down")
			mainMenu.Next();
		else if (event.gameEvent == "menu_select")
		{
			//TraceLog(LOG_INFO, "Selected \"%s\"", mainMenu.GetSelections()[mainMenu.GetSelected()].c_str());
			application->GetStateMachine()->PushNew<TestState>();
		}
			
	}
}

void MainMenuState::OnPush()
{

}

void MainMenuState::OnPop()
{

}
