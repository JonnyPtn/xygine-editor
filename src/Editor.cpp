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
#include <xyginext/ecs/components/Camera.hpp>
#include <xyginext/ecs/components/Transform.hpp>

#include "States.hpp"
#include "OpenState.hpp"
#include "NewState.hpp"
#include "SpriteEditState.hpp"
#include "Messages.hpp"
#include "imgui_tabs.h"
#include "imgui-SFML.h"

//absolute paths to open documents
std::vector<std::string> openDocuments;
    
const std::string applicationName = "xygine Editor";

Editor::Editor()
    : xy::App   (/*sf::ContextSettings(0, 0, 0, 3, 2, sf::ContextSettings::Core)*/),
    m_stateStack({ *getRenderWindow(), *this })
{
    xy::App::setApplicationName(applicationName);
    // Check for any previously open documents
    std::ifstream doc(xy::FileSystem::getConfigDirectory(applicationName));
    if (doc.good())
    {
        std::string file;
        while(doc >> file)
        {
            auto msg = getMessageBus().post<std::string>(Messages::NEW_WORKING_FILE);
            *msg = file;
        }
    }
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
                
            // ctrl + n for new
            case sf::Keyboard::N:
                if (evt.key.control)
                    m_stateStack.pushState(States::NEW);
                break;
            
            // ctrl + x for exit
            case sf::Keyboard::X:
                if (evt.key.control)
                    quit();
                break;
                
            default:
                break;
                
        }
        break;
            
        case sf::Event::Closed:
            quit();
            break;
    }
    m_stateStack.handleEvent(evt);
}

void Editor::handleMessage(const xy::Message& msg)
{
    if (msg.id == Messages::NEW_WORKING_FILE)
        openDocuments.push_back(msg.getData<std::string>());
    
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
    
    // File menu
    if (ImGui::BeginMenu("File"))
    {
        if (ImGui::MenuItem("New", "ctrl+n"))  m_stateStack.pushState(States::NEW);
        if (ImGui::MenuItem("Open", "ctrl+o")) m_stateStack.pushState(States::OPEN);
        ImGui::Separator();
        if (ImGui::MenuItem("Exit", "ctrl+x")) quit();
        ImGui::EndMenu();
    }
    
    // View menu
    static bool showStyleEditor = false;
    if (ImGui::BeginMenu("View"))
    {
        if (ImGui::BeginMenu("Windows"))
        {
            ImGui::MenuItem("Editor Style", nullptr, &showStyleEditor);
            ImGui::EndMenu();
        }
        ImGui::EndMenu();
    }
    
    if (showStyleEditor)
    {
        ImGui::Begin("Editor Style");
        ImGui::ShowStyleEditor();
        ImGui::End();
    }
    
    xy::Nim::setNextWindowPosition(0,ImGui::GetWindowHeight());
    xy::Nim::setNextWindowSize(getRenderWindow()->getSize().x,0.f);
    
    bool open(true);
    ImGui::Begin("editor", &open, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
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
    m_stateStack.registerState<OpenState>(States::OPEN);
    m_stateStack.registerState<SpriteEditState>(States::SPRITE_EDIT);
}
