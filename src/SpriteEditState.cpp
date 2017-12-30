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

int SpriteEditState::m_instanceCount = 0;

constexpr int InputBufMax = 1024;

SpriteEditState::SpriteEditState(xy::StateStack& stateStack, Context context, xy::Scene& previewScene) :
xy::State(stateStack, context),
m_initialised(false),
m_selectedSpriteName("Select a sprite"),
m_selectedAnimName("Select an animation"),
m_unsavedChanges(false),
m_previewScene(previewScene),
m_previewEntity(0)
{
    m_id = m_instanceCount++;
}

bool SpriteEditState::handleEvent(const sf::Event &evt) {
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
}

bool SpriteEditState::update(float dt)
{
    // Update anim previews
    for (auto& a : m_animations)
    {
        a.second.frametime -= dt;
        if (a.second.frametime < 0 && a.second.anim.frameCount > 0)
        {
            XY_ASSERT(a.second.anim.framerate > 0, "Illegal Frame Rate");
            XY_ASSERT(a.second.anim.frameCount > 0, "Illegal Frame Count");
            a.second.frametime += (1.f / a.second.anim.framerate);
            
            a.second.frame = (a.second.frame + 1) % a.second.anim.frameCount;
            
            a.second.sprite.setTextureRect(sf::IntRect(a.second.anim.frames[a.second.frame]));
        }
    }
    return true;
}

void SpriteEditState::draw() {
    
    bool open(true);
    
    bool selected = ImGui::TabItem(m_name.c_str(), &open, m_unsavedChanges ? ImGuiTabItemFlags_UnsavedDocument : 0);
    
    
    if (!open)
        requestStackPop();
    
    if (!selected)
        return;
    
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
        bool modifiedRect = ImGui::InputInt("left",&rect.left)
            || ImGui::InputInt("top",&rect.top)
            || ImGui::InputInt("width", &rect.width)
        || ImGui::InputInt("height", &rect.height);
        
        if (modifiedRect)
        {
            sprite.setTextureRect(sf::FloatRect(rect));
            m_sheet.setSprite(m_selectedSpriteName, sprite);
            updatePreview();
            m_unsavedChanges = true;
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
        
        
        // Save button
        if (xy::Nim::button("Save"))
        {
            m_sheet.saveToFile(m_path);
            m_unsavedChanges = false;
        }
    }
    
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
    
    // also bad...
    if (m_selectedAnimName != "Select an animation")
    {
        m_previewEntity.addComponent<xy::SpriteAnimation>().play(m_sheet.getAnimationIndex(m_selectedAnimName, m_selectedSpriteName) );
    }
}

xy::StateID SpriteEditState::stateID() const {
    return States::SPRITE_EDIT;
}
