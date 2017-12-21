//
//  SplashState.cpp
//  xygine-editor
//
//  Created by Jonny Paton on 20/12/2017.
//

#include "OpenState.hpp"

#include <xyginext/gui/Gui.hpp>
#include <xyginext/core/FileSystem.hpp>
#include <xyginext/core/App.hpp>
#include <xyginext/core/MessageBus.hpp>

#include "imgui.h"

#include "States.hpp"
#include "Messages.hpp"

OpenState::OpenState(xy::StateStack &stateStack, xy::State::Context context) :
xy::State(stateStack, context)
{
    
}

bool OpenState::handleEvent(const sf::Event &evt) { 
    
}

void OpenState::handleMessage(const xy::Message &) { 

}

bool OpenState::update(float dt) { 
    
}

void OpenState::draw()
{
    ImGui::ShowTestWindow();
    
    xy::Nim::begin("Open");
    
    static std::string path = "";
    
    const int bufSize = 1024;
    static char inputBuf[1024];
    ImGui::ShowTestWindow();
    ImGui::InputText("Sprite name: ", inputBuf, bufSize);
    
    if (xy::Nim::button("Browse"))
    {
        path = xy::FileSystem::nativeOpenFile();
        requestStackPop();
        requestStackPush(States::SPRITE_EDIT);
        
        auto msg = getContext().appInstance.getMessageBus().post<std::string>(Messages::NEW_WORKING_FILE);
        *msg = path;
    }
    
    xy::Nim::end(); // New Sprite
}

xy::StateID OpenState::stateID() const {
    return States::OPEN;
}
