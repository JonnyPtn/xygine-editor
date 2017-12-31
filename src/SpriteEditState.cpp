//
//  SpriteEditState.cpp
//  xygine-editor
//
//  Created by Jonny Paton on 20/12/2017.
//

#include "SpriteEditState.hpp"
#include "States.hpp"
#include "Messages.hpp"
#include <xyginext/core/FileSystem.hpp>
#include <xyginext/gui/Gui.hpp>
#include <xyginext/core/Message.hpp>
#include <SFML/Graphics.hpp>
#include <xyginext/core/App.hpp>
#include "imgui.h"
#include "imgui-SFML.h"
#include "imgui_tabs.h"
#include <xyginext/ecs/Scene.hpp>
#include <xyginext/ecs/components/Transform.hpp>
#include <xyginext/ecs/components/Drawable.hpp>
#include <xyginext/ecs/components/Camera.hpp>
#include <xyginext/ecs/systems/RenderSystem.hpp>
#include <xyginext/ecs/systems/SpriteSystem.hpp>
#include <xyginext/ecs/systems/SpriteAnimator.hpp>
#include <xyginext/ecs/systems/CameraSystem.hpp>

int SpriteEditState::m_instanceCount = 0;

constexpr float PreviewWidth = 0.6f;

constexpr int InputBufMax = 1024;

SpriteEditState::SpriteEditState(xy::StateStack& stateStack, Context context) :
xy::State(stateStack, context),
m_initialised(false),
m_selectedSpriteName("Select a sprite"),
m_selectedAnimName("Select an animation"),
m_unsavedChanges(false),
m_previewScene(context.appInstance.getMessageBus()),
m_previewEntity(0),
m_draggingPreview(false)
{
    m_id = m_instanceCount++;
    
    auto& mb = getContext().appInstance.getMessageBus();
    m_previewScene.addSystem<xy::CameraSystem>(mb);
    m_previewScene.addSystem<xy::SpriteAnimator>(mb);
    m_previewScene.addSystem<xy::RenderSystem>(mb);
    m_previewScene.addSystem<xy::SpriteSystem>(mb);
    
    // Add camera entity
    m_camEntity = m_previewScene.createEntity();
    m_camEntity.addComponent<xy::Camera>().setViewport({1.f - PreviewWidth, 0.f, PreviewWidth, 1.f});
    auto winSize = context.appInstance.getRenderWindow()->getSize();
    m_camEntity.getComponent<xy::Camera>().setView({winSize.x * PreviewWidth, static_cast<float>(winSize.y)});
    m_camEntity.addComponent<xy::Transform>();
    m_previewScene.setActiveCamera(m_camEntity);
}

bool SpriteEditState::handleEvent(const sf::Event &evt)
{
    switch (evt.type)
    {
    }
}

void SpriteEditState::handleMessage(const xy::Message & msg)
{
    switch(msg.id)
    {
        case Messages::NEW_WORKING_FILE:
            if (!m_initialised)
            {
                m_path = msg.getData<std::string>();
                m_name = xy::FileSystem::getFileName(m_path);
                m_sheet.loadFromFile(m_path, m_texture);
                
                m_initialised = true;
            }
            break;
    }
    m_previewScene.forwardMessage(msg);
}

bool SpriteEditState::update(float dt)
{
    m_previewScene.update(dt);
}

void SpriteEditState::draw() {
    
    bool open(true);
    
    auto window = getContext().appInstance.getRenderWindow();
    
    bool selected = ImGui::TabItem(m_name.c_str(), &open, m_unsavedChanges ? ImGuiTabItemFlags_UnsavedDocument : 0);
    
    
    if (!open)
        requestStackPop();
    
    if (!selected)
        return;
    
    ImGui::SetNextWindowPos({ImGui::GetWindowPos().x, ImGui::GetWindowPos().y + ImGui::GetWindowHeight()});
    ImGui::SetNextWindowSize({window->getSize().x * (1.f - PreviewWidth),static_cast<float>(window->getSize().y)});

    
    ImGui::Begin("Properties" , nullptr,ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);

    
    // Show the texture being used first
    auto texPath = m_sheet.getTexturePath();
    std::array<char, InputBufMax> texPathInput = {{0}};
    texPath.copy(texPathInput.begin(),texPath.size());
    if (ImGui::InputText("Texture", texPathInput.data(), texPathInput.size()))
    {
        m_sheet.setTexturePath(std::string(texPathInput.data()));
        m_unsavedChanges = true;
    }
    ImGui::SameLine();
    if (ImGui::Button("Browse"))
    {
        auto path = xy::FileSystem::openFileDialogue();
        // Maybe xygine should handle this?
        m_sheet.setTexturePath(xy::FileSystem::getRelativePath(path,m_path.substr(0,m_path.find_last_of('/'))));
    }
    
    auto& tex = m_texture.get(xy::FileSystem::getFilePath(m_path) + m_sheet.getTexturePath());
    if (ImGui::BeginCombo("Sprites", m_selectedSpriteName.c_str()))
    {
        for (auto& spr : m_sheet.getSprites())
        {
            bool sprSelected(false);
            ImGui::Selectable(spr.first.c_str(), &sprSelected);
            if (sprSelected)
            {
                m_selectedSpriteName = spr.first;
                updatePreview();
            }
        }
        ImGui::EndCombo();
    }
    
    // bad...
    if (m_selectedSpriteName != "Select a sprite")
    {
        auto sprite = m_sheet.getSprite(m_selectedSpriteName);
        
        // Texture rect
        ImGui::Text("Texture Rect:");
        auto rect = sf::IntRect(sprite.getTextureRect());
        bool left = ImGui::InputInt("left",&rect.left);
        bool top = ImGui::InputInt("top",&rect.top);
        bool width = ImGui::InputInt("width", &rect.width);
        bool height = ImGui::InputInt("height", &rect.height);
        
        if (left || top || width || height)
        {
            sprite.setTextureRect(sf::FloatRect(rect));
            updatePreview();
            m_unsavedChanges = true;
        }
        ImVec4 col= sprite.getColour();
        if (ImGui::ColorEdit3("Colour", (float*)&col))
        {
            sprite.setColour(col);
            m_sheet.setSprite(m_selectedSpriteName, sprite);
            
            updatePreview();
        }
        
        // Animations
        if (ImGui::BeginCombo("Animations", m_selectedAnimName.c_str()))
        {
            auto& anims = sprite.getAnimations();
            for (auto& anim : anims)
            {
                bool animSelected(false);
                ImGui::Selectable(anim.id.data(), &animSelected);
                if (animSelected)
                {
                    m_selectedAnimName = std::string(anim.id.data());
                    updatePreview();
                }
            }
            ImGui::EndCombo();
        }
        
        // also bad...
        if (m_selectedAnimName != "Select an animation")
        {
            
        }
        
        // Save button
        if (xy::Nim::button("Save"))
        {
            m_sheet.saveToFile(m_path);
            m_unsavedChanges = false;
        }
    }
    ImGui::End();
    window->draw(m_previewScene);
}

void SpriteEditState::updatePreview()
{
    // Update preview with new sprite
    if (m_previewEntity > 0)
        m_previewScene.destroyEntity(m_previewEntity);
    
    m_previewEntity = m_previewScene.createEntity();
    m_previewEntity.addComponent(m_sheet.getSprite(m_selectedSpriteName));
    m_previewEntity.addComponent<xy::Transform>();
    m_previewEntity.addComponent<xy::Drawable>();
    
    // Adjust the camera so the sprite fills the view
    auto view = m_camEntity.getComponent<xy::Camera>().getView();
    auto size = m_previewEntity.getComponent<xy::Sprite>().getSize();
    
    auto zoom = std::min(float(view.x)/float(size.x), float(view.y)/float(size.y));
    
    m_camEntity.getComponent<xy::Camera>().zoom(zoom);
    m_camEntity.getComponent<xy::Transform>().setPosition(size.x/2,size.y/2);
    
    // also bad...
    if (m_selectedAnimName != "Select an animation")
    {
        m_previewEntity.addComponent<xy::SpriteAnimation>().play(m_sheet.getAnimationIndex(m_selectedAnimName, m_selectedSpriteName) );
    }
}

xy::StateID SpriteEditState::stateID() const {
    return States::SPRITE_EDIT;
}
