
#include "../State.h"
#include "../UI/Window.h"
#include "../UI/Menu.h"
#include "../UI/UIElement.h"

class MainMenuState : public State
{
protected:
    Font font;
    Window mainMenuWindow;
    Menu mainMenu;
    UIElement mainMenuUIE;
public:
	MainMenuState(Application *app);
	void OnLoad() override;
    void OnUpdate(double delta) override;
    void OnDraw() override;
    void OnDebugDraw() override;
    void OnEvent(Event event) override;
    void OnPush() override;
    void OnPop() override;
};