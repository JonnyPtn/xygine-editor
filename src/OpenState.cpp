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
    m_windowSize = context.renderWindow.getSize();
}

bool OpenState::handleEvent(const sf::Event &evt) { 
    
}

void OpenState::handleMessage(const xy::Message &msg) {
    
    if (msg.id == xy::Message::WindowMessage)
    {
        const auto& data = msg.getData<xy::Message::WindowEvent>();
        m_windowSize = {data.width, data.height};
    }
}

bool OpenState::update(float dt) { 
    
}

void OpenState::draw()
{
    // Open covers entire window and centers
    xy::Nim::setNextWindowSize(ImGui::GetWindowWidth(),ImGui::GetWindowHeight());
    xy::Nim::setNextWindowPosition(0, 0);
    xy::Nim::begin("Open");
    
    static std::string path = "";
    
    const int bufSize = 1024;
    static char inputBuf[1024];
    ImGui::ShowTestWindow();
    ImGui::InputText("File name", inputBuf, bufSize);
    
    if (xy::Nim::button("Browse"))
    {
        path = xy::FileSystem::openFileDialogue();
        requestStackPop();
        requestStackPush(States::SPRITE_EDIT);
        
        auto msg = getContext().appInstance.getMessageBus().post<std::string>(Messages::NEW_WORKING_FILE);
        *msg = path;
    }
    
    xy::Nim::end(); // "Open"
}

xy::StateID OpenState::stateID() const {
    return States::OPEN;
}
