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

OpenState::OpenState(xy::StateStack &stateStack, xy::State::Context context, sf::IntRect editorRect) :
xy::State(stateStack, context),
m_windowRect(editorRect)
{
}

bool OpenState::handleEvent(const sf::Event &evt) { 
    
}

void OpenState::handleMessage(const xy::Message &msg) {
    
    if (msg.id == Messages::NEW_WORKING_RECT)
    {
        const auto& data = msg.getData<sf::IntRect>();
        m_windowRect = data;
    }
}

bool OpenState::update(float dt) { 
    
}

void OpenState::draw()
{
    // Open covers entire window and centers
    xy::Nim::setNextWindowSize(m_windowRect.width, m_windowRect.height);
    xy::Nim::setNextWindowPosition(m_windowRect.left, m_windowRect.top);
    bool open;
    ImGui::Begin("Open", &open, ImGuiWindowFlags_NoTitleBar);
    
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
