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
    return false;
    
}

void OpenState::handleMessage(const xy::Message &msg) {
    
}

bool OpenState::update(float dt) { 
    
}

void OpenState::draw()
{
    // Open covers entire window and centers
    ImGui::SetNextWindowPosCenter();
    bool open(true);
    ImGui::Begin("Open", &open, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);
    
    if (!open)
        requestStackPop();
    
    static std::string path = "";
    
    const int bufSize = 1024;
    static char inputBuf[1024];
    ImGui::InputText("File name", inputBuf, bufSize);
    
    if (xy::Nim::button("Browse"))
    {
        path = xy::FileSystem::openFileDialogue();
        if (path.length())
        {
            requestStackPop();
            
            auto msg = getContext().appInstance.getMessageBus().post<std::string>(Messages::OPEN_PROJECT);
            *msg = path;
            
            requestStackPush(States::PROJECT_EDIT);
        }
    }
    
    xy::Nim::end(); // "Open"
}

xy::StateID OpenState::stateID() const {
    return States::OPEN;
}
