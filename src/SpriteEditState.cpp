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
#include "imgui.h"
#include "imgui-SFML.h"
#include "imgui_tabs.h"

int SpriteEditState::m_instanceCount = 0;

SpriteEditState::SpriteEditState(xy::StateStack& stateStack, Context context) :
xy::State(stateStack, context),
m_initialised(false),
m_unsavedChanges(false)
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
    
    auto& tex = m_texture.get(xy::FileSystem::getFilePath(m_path) + m_sheet.getTexturePath());
    if (ImGui::TreeNode(("Sheet texture: " + m_sheet.getTexturePath()).c_str()))
    {
        if (ImGui::Button("Browse"))
        {
            auto path = xy::FileSystem::openFileDialogue();
            m_sheet.setTexturePath(xy::FileSystem::getRelativePath(path, xy::FileSystem::getFilePath(m_path)));
            m_unsavedChanges = true;
        }
        ImGui::Image(tex);
        ImGui::TreePop();
    }
    
    // Show sprite details
    int spriteCount(0);
    for (auto& spr : m_sheet.getSprites())
    {
        if (ImGui::TreeNode(spr.first.c_str()))
        {
            sf::Sprite sprite;
            sprite.setTexture(tex);
            sprite.setTextureRect(sf::IntRect(spr.second.getTextureRect()));
            ImGui::Image(sprite);
            
            // Show it's animations
            auto& anims = spr.second.getAnimations();
            for (auto i = 0u; i < spr.second.getAnimationCount(); i++)
            {
                auto& anim = anims[i];
                if (ImGui::TreeNode(("Animation " + std::string(anim.id.data()) + "##" + spr.first).c_str()))
                {
                    if (!m_animations.count(spr.first + anim.id.data()))
                    {
                        sf::Sprite previewSprite(tex);
                        previewSprite.setTextureRect(sf::IntRect(spr.second.getTextureRect()));
                        m_animations[spr.first + anim.id.data()] = {anim,previewSprite,0.f,0};
                    }
                    
                    ImGui::Image(m_animations[spr.first + anim.id.data()].sprite);
                    ImGui::TreePop();
                }
            }
            ImGui::TreePop();
        }
        ++spriteCount;
    }
    
    // Save button
    if (xy::Nim::button("Save"))
    {
        m_sheet.saveToFile(m_path);
        m_unsavedChanges = false;
    }
    
}

xy::StateID SpriteEditState::stateID() const {
    return States::SPRITE_EDIT;
}
