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
#include "ProjectEditState.hpp"
#include "Messages.hpp"
#include "imgui-SFML.h"
#include "imgui_dock.hpp"
    
const std::string applicationName = "xygine Editor";


Editor::Editor()
    : xy::App   (/*sf::ContextSettings(0, 0, 0, 3, 2, sf::ContextSettings::Core)*/),
    m_stateStack({ *getRenderWindow(), *this })
{
    xy::App::setApplicationName(applicationName);
    
    // Open our config file
    m_editorConfigPath = xy::FileSystem::getConfigDirectory(applicationName) + "EditorSettings.cfg";
    if (m_editorConfig.loadFromFile(m_editorConfigPath))
    {
        // settings exist, load previous project
        for (auto& p : m_editorConfig.getProperties())
        {
            if (p.getName() == "PreviousProject")
            {
                auto msg = getMessageBus().post<std::string>(Messages::OPEN_PROJECT);
                *msg = p.getValue<std::string>();
                m_stateStack.pushState(States::PROJECT_EDIT);
            }
        }
    }
    else
    {
        // config not found - create it
        m_editorConfig.save(m_editorConfigPath);
    }
    
    getRenderWindow()->setVerticalSyncEnabled(false);
    getRenderWindow()->setFramerateLimit(60);
    
    // Load the style
    xy::Nim::Style style;
    if (style.loadFromFile(xy::FileSystem::getConfigDirectory(getApplicationName()) + "style.cfg"))
    {
        xy::Nim::setStyle(style);
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
            ImGui::ShutdownDock();
            quit();
            break;
    }
    m_stateStack.handleEvent(evt);
}

void Editor::handleMessage(const xy::Message& msg)
{
    // Listen for changes in active project, so we can restore correctly
    if (msg.id == Messages::OPEN_PROJECT)
    {
        auto& path = msg.getData<std::string>();
        
        // Store in our config file
        xy::ConfigProperty* p;
        if (p = m_editorConfig.findProperty("PreviousProject"))
            p->setValue(path);
        else
            m_editorConfig.addProperty("PreviousProject", path);
        m_editorConfig.save(m_editorConfigPath);
    }
    m_stateStack.handleMessage(msg);
}

void Editor::updateApp(float dt)
{
    m_stateStack.update(dt);
}

void Editor::draw()
{
    // Menu bar
    if (ImGui::BeginMainMenuBar())
    {
        
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
        static bool styleEditorShowing = false;
        if (ImGui::BeginMenu("View"))
        {
            if (ImGui::BeginMenu("Windows"))
            {
                ImGui::MenuItem("Editor Style", nullptr, &styleEditorShowing);
                ImGui::EndMenu();
            }
            ImGui::EndMenu();
        }
        
        if (styleEditorShowing)
        {
            showStyleEditor();
        }
        
        // Show fps in menu bar
        ImGui::SetCursorPosX(ImGui::GetWindowWidth() - 60);
        ImGui::Text(std::string("FPS: " + std::to_string(int(std::round(ImGui::GetIO().Framerate)))).c_str()); // wuzn't me
        
        m_stateStack.draw();
        
        ImGui::EndMainMenuBar();
    }
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
    m_stateStack.registerState<ProjectEditState>(States::PROJECT_EDIT);
}

void Editor::showStyleEditor()
{
    xy::Nim::begin("Style Editor");
    
    // You could cache the style to improve performance if needed
    auto style = xy::Nim::getStyle();
    
    xy::Nim::slider("Alpha", style.Alpha, 0.f, 1.f);
    xy::Nim::checkbox("Antialiased fill", &style.AntiAliasedFill);
    xy::Nim::checkbox("Antialiased lines", &style.AntiAliasedLines);
    xy::Nim::slider("Window rounding", style.WindowRounding, 0.f, 100.f);
    
    xy::Nim::setStyle(style);
    
    if (xy::Nim::button("Save"))
    {
        style.saveToFile(xy::FileSystem::getConfigDirectory(getApplicationName()) + "style.cfg");
    }
    xy::Nim::end();
}
