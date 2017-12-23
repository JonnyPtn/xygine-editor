/*********************************************************************
(c) Matt Marchant 2017
http://trederia.blogspot.com

xygineXT - Zlib license.

This software is provided 'as-is', without any express or
implied warranty. In no event will the authors be held
liable for any damages arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute
it freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented;
you must not claim that you wrote the original software.
If you use this software in a product, an acknowledgment
in the product documentation would be appreciated but
is not required.

2. Altered source versions must be plainly marked as such,
and must not be misrepresented as being the original software.

3. This notice may not be removed or altered from any
source distribution.
*********************************************************************/

#include "Editor.hpp"

#include <SFML/Window/Event.hpp>

#include <xyginext/gui/Gui.hpp>

#include "States.hpp"
#include "OpenState.hpp"
#include "NewState.hpp"
#include "SpriteEditState.hpp"
#include "Messages.hpp"
#include "imgui_tabs.h"
#include "imgui-SFML.h"

Editor::Editor()
    : xy::App   (/*sf::ContextSettings(0, 0, 0, 3, 2, sf::ContextSettings::Core)*/),
    m_stateStack({ *getRenderWindow(), *this })
{
}

//private
void Editor::handleEvent(const sf::Event& evt)
{
    // Check for hot keys
    switch(evt.type)
    {
        case sf::Event::KeyPressed:
            switch (evt.key.code)
        {
            // ctrl + o to open
            case sf::Keyboard::O:
                if (evt.key.control)
                    m_stateStack.pushState(States::OPEN);
                break;
        }
    }
    m_stateStack.handleEvent(evt);
}

void Editor::handleMessage(const xy::Message& msg)
{
    m_stateStack.handleMessage(msg);
}

void Editor::updateApp(float dt)
{
    m_stateStack.update(dt);
}

void Editor::draw()
{
    // Menu bar
    ImGui::BeginMainMenuBar();
    if (ImGui::BeginMenu("File"))
    {
        if (ImGui::MenuItem("New"))  m_stateStack.pushState(States::NEW);
        if (ImGui::MenuItem("Open", "ctrl+o")) m_stateStack.pushState(States::OPEN);
        ImGui::Spacing();
        if (ImGui::MenuItem("Exit")) quit();
        ImGui::EndMenu();
    }
    
    // Check for main menu bar change in size
    auto h = ImGui::GetWindowHeight();
    auto Wh = getRenderWindow()->getSize().y;
    if (Wh - h != m_editorWindowRect.height)
    {
        m_editorWindowRect.left = 0;
        m_editorWindowRect.top = h;
        m_editorWindowRect.width = getRenderWindow()->getSize().x;
        m_editorWindowRect.height = Wh - h;
        const auto& msg = getMessageBus().post<sf::IntRect>(Messages::NEW_WORKING_RECT);
        *msg = m_editorWindowRect;
    }
    
    xy::Nim::setNextWindowSize(m_editorWindowRect.width,m_editorWindowRect.height);
    xy::Nim::setNextWindowPosition(m_editorWindowRect.left,m_editorWindowRect.top);
    
    bool open(true);
    ImGui::Begin("editor", &open, ImGuiWindowFlags_NoTitleBar);
    ImGui::BeginTabBar("Resources");
    m_stateStack.draw();
    ImGui::EndTabBar();
    ImGui::End();
    
    ImGui::EndMainMenuBar();
}

void Editor::initialise()
{
    registerStates();


    getRenderWindow()->setKeyRepeatEnabled(false);
}

void Editor::finalise()
{
    m_stateStack.clearStates();
    m_stateStack.applyPendingChanges();
}

void Editor::registerStates()
{
    m_stateStack.registerState<NewState>(States::NEW);
    m_stateStack.registerState<OpenState>(States::OPEN, m_editorWindowRect);
    m_stateStack.registerState<SpriteEditState>(States::SPRITE_EDIT);
}
