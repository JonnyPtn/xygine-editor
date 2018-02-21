//
//  SpriteEditState.cpp
//  xygine-editor
//
//  Created by Jonny Paton on 20/12/2017.
//

#include "ProjectEditState.hpp"
#include "States.hpp"
#include "Messages.hpp"
#include <xyginext/core/FileSystem.hpp>
#include <xyginext/gui/Gui.hpp>
#include <xyginext/core/Message.hpp>
#include <SFML/Graphics.hpp>
#include <xyginext/core/App.hpp>
#include "imgui.h"
#include "imgui-SFML.h"
#include "imgui_dock.hpp"
#include "imgui_tabs.h"
#include <xyginext/ecs/Scene.hpp>
#include <xyginext/ecs/components/Transform.hpp>
#include <xyginext/ecs/components/Drawable.hpp>
#include <xyginext/ecs/components/Camera.hpp>
#include <xyginext/ecs/systems/RenderSystem.hpp>
#include <xyginext/ecs/systems/ParticleSystem.hpp>
#include <xyginext/ecs/systems/ParticleSystem.hpp>
#include <xyginext/ecs/systems/SpriteSystem.hpp>
#include <xyginext/ecs/systems/SpriteAnimator.hpp>
#include <xyginext/ecs/systems/CameraSystem.hpp>

#include "SpriteAsset.hpp"

//err...
#include <unistd.h>

const sf::Vector2u PreviewSize(800,600);

ProjectEditState::ProjectEditState(xy::StateStack& stateStack, Context context) :
xy::State(stateStack, context),
m_unsavedChanges(false),
m_previewScene(context.appInstance.getMessageBus()),
m_draggingPreview(false),
m_currentProject("")
{
    auto& mb = context.appInstance.getMessageBus();
    
    // Add required systems for preview scene
    m_previewScene.addSystem<xy::CameraSystem>(mb);
    m_previewScene.addSystem<xy::SpriteAnimator>(mb);
    m_previewScene.addSystem<xy::RenderSystem>(mb);
    m_previewScene.addSystem<xy::SpriteSystem>(mb);
    m_previewScene.addSystem<xy::ParticleSystem>(mb);
    
    // Add camera entity
    m_previewCamera = m_previewScene.createEntity();
    m_previewCamera.addComponent<xy::Camera>();
    m_previewCamera.addComponent<xy::Transform>();
    m_previewScene.setActiveCamera(m_previewCamera);
    
    // Create the preview window
    m_previewWindow.create(context.appInstance.getVideoSettings().VideoMode, "Preview");
    
    // Update the camera view, otherwise everything is warped
    m_previewCamera.getComponent<xy::Camera>().setView(sf::Vector2f(m_previewWindow.getSize()));
}

bool ProjectEditState::handleEvent(const sf::Event &evt)
{
    switch (evt.type)
    {
    }
    m_previewScene.forwardEvent(evt);
}

void ProjectEditState::handleMessage(const xy::Message & msg)
{
    switch(msg.id)
    {
        case Messages::OPEN_PROJECT:
        {
            auto file = msg.getData<std::string>();
            
            // change working directory to project folder
            chdir(xy::FileSystem::getFilePath(file).c_str());
            m_currentProject.loadFromFile(file);
            break;
        }
            
        case Messages::ASSET_SELECTED:
        {
            // Check the type, based on extension. I know this sucks.
            auto file = msg.getData<std::string>();
            
            if (file.find(".spt"))
            {
                m_selectedAssetType = AssetType::Sprite;
                m_selectedAsset.reset(new SpriteAsset(file, m_previewScene));
            }
            else if (file.find(".tmx"))
                m_selectedAssetType = AssetType::TileMap;
            else if (file.find(".xyp"))
                m_selectedAssetType = AssetType::ParticleEmitter;
            else if (file.find(".wav"))
                m_selectedAssetType = AssetType::Sound;
            else if (file.find(".png"))
                m_selectedAssetType = AssetType::Texture;
            else if (file.find(".jpg"))
                m_selectedAssetType = AssetType::Texture;
            
            break;
        }
    }
    m_previewScene.forwardMessage(msg);
}

bool ProjectEditState::update(float dt)
{
    // First check the preview window for events
    sf::Event ev;
    while(m_previewWindow.pollEvent(ev))
    {
        switch(ev.type)
        {
            case sf::Event::MouseButtonPressed:
            {
                m_draggingPreview = true;
                break;
            }
            case sf::Event::MouseButtonReleased:
            {
                m_draggingPreview = false;
                break;
            }
            case sf::Event::MouseMoved:
            {
                // If we're dragging preview, translate camera appropriately
                if (m_draggingPreview)
                {
                    auto dx = ev.mouseMove.x - m_lastMousePos.x;
                    auto dy = ev.mouseMove.y - m_lastMousePos.y;
                    m_previewCamera.getComponent<xy::Transform>().move(-dx,-dy);
                    auto pos = m_previewCamera.getComponent<xy::Transform>().getPosition();
                }
                m_lastMousePos = {ev.mouseMove.x, ev.mouseMove.y};
                break;
            }
        }
    }
    
    m_previewScene.update(dt);
}

void ProjectEditState::draw()
{
    auto menuBarHeight = ImGui::GetWindowHeight(); // For use below
    
    // Wrap all other states in a dockspace, so dockable windows can be used
    auto windowSize = getContext().appInstance.getRenderWindow()->getSize();
    
    // Because this is called from within BeginMainMenuBar we can query imgui window height to get
    // the height of the menu bar
    ImGui::SetNextWindowSize({static_cast<float>(windowSize.x) ,windowSize.y - menuBarHeight});
    ImGui::SetNextWindowPos({0,menuBarHeight});
    ImGui::Begin("DockSpace", nullptr, ImGuiWindowFlags_NoTitleBar);
    ImGui::BeginDockspace();
    
    drawAssetBrowser();
    if (m_selectedAsset)
    {
        m_selectedAsset->drawProperties();
    }
    
    ImGui::EndDockspace();
    ImGui::End();
    
    // Draw the preview window
    m_previewWindow.clear();
    m_previewWindow.draw(m_previewScene);
    m_previewWindow.display();
}

void ProjectEditState::drawAssetBrowser()
{
    if (ImGui::BeginDock("Asset Browser"))
    {
        std::function<void(std::string)> imFileTreeRecurse = [&](std::string path)
        {
            // List directories, recurse if selected
            for (auto& dir : xy::FileSystem::listDirectories(path))
            {
                if (ImGui::TreeNode(dir.c_str()))
                {
                    imFileTreeRecurse(path + "/" + dir);
                    ImGui::TreePop();
                }
            }
            
            // List files, broadcast message if selected
            for (auto& file : xy::FileSystem::listFiles(path))
            {
                auto selected = m_selectedFile == file;
                if (ImGui::Selectable(file.c_str(), &selected))
                {
                    auto msg = getContext().appInstance.getMessageBus().post<std::string>(Messages::ASSET_SELECTED);
                    *msg = m_selectedFile = xy::FileSystem::getFilePath(m_currentProject.getFilePath()) + "/" +  path + "/" + file;
                }
            }
        };
        
        imFileTreeRecurse("assets");
    }
    ImGui::EndDock();
}

xy::StateID ProjectEditState::stateID() const {
    return States::PROJECT_EDIT;
}
