//
//  SpriteAsset.cpp
//  xygine-editor
//
//  Created by Jonny Paton on 20/02/2018.
//

#include "SpriteAsset.hpp"
#include "imgui_dock.hpp"
#include "imgui.h"
#include "imgui-SFML.h"

#include "Editor.hpp"

#include <xyginext/ecs/systems/SpriteSystem.hpp>
#include <xyginext/ecs/systems/RenderSystem.hpp>
#include <xyginext/ecs/systems/CameraSystem.hpp>
#include <xyginext/ecs/systems/SpriteAnimator.hpp>

#include <xyginext/ecs/components/Camera.hpp>
#include <xyginext/ecs/components/Transform.hpp>
#include <xyginext/ecs/components/Drawable.hpp>
#include <xyginext/ecs/components/SpriteAnimation.hpp>

#include <array>
#include <unistd.h>

const sf::Vector2f PreviewSize(400,400);

SpriteAsset::SpriteAsset(const std::string& path, xy::Scene& previewScene) :
Asset(),
m_assetPath(path),
m_previewScene(previewScene)
{
    // Because xy::Spritesheet automatically prepends the bundle path, we need to go relative to that
    auto resPath = xy::FileSystem::getResourcePath();
    resPath = resPath.substr(0,resPath.find_last_of("/"));
    auto relPath = xy::FileSystem::getRelativePath(path,resPath);
    m_sheet.loadFromFile(relPath, m_textures);
    
    // Because the texture resource will also try to load from our resource path, we need to manually
    // load the texture using the correct path
    const size_t bufSize = 1024;
    std::array<char,bufSize> buf = {{0}};
    getcwd(buf.data(), bufSize);
    auto texPath = std::string(buf.data()) + "/" + m_sheet.getTexturePath();
    texPath = xy::FileSystem::getRelativePath(texPath, resPath);
    m_textures.get(texPath);
    
    // Our entity for the preview
    m_previewSprite = m_previewScene.createEntity();
    m_previewSprite.addComponent<xy::Sprite>();
    m_previewSprite.addComponent<xy::Transform>();
    m_previewSprite.addComponent<xy::Drawable>();
    m_previewSprite.addComponent<xy::SpriteAnimation>();
}

void SpriteAsset::drawProperties()
{
    if (ImGui::BeginDock("Spritesheet Properties"))
    {
        
        // Path to the texture
        const size_t bufSize = 1024;
        std::array<char, bufSize> texturePath = {{0}};
        m_sheet.getTexturePath().copy(texturePath.begin(), m_sheet.getTexturePath().length());
        ImGui::InputText("Texture", texturePath.data(), bufSize);
        auto resPath = xy::FileSystem::getResourcePath();
        resPath = resPath.substr(0,resPath.find_last_of("/"));
        
        // Texture preview, along with texture rect indicator
        std::array<char,bufSize> buf = {{0}};
        getcwd(buf.data(), bufSize);
        auto texPath = std::string(buf.data()) + "/" + m_sheet.getTexturePath();
        texPath = xy::FileSystem::getRelativePath(texPath, resPath);
        auto& tex = m_textures.get(texPath);
        sf::IntRect texRect = sf::IntRect(m_previewSprite.getComponent<xy::Sprite>().getTextureRect());
        ImGui::BeginChild("Texture Preview",{200,200});
        ImGui::DrawRect(sf::FloatRect(texRect), sf::Color::Red);
        ImGui::Image(tex);
        ImGui::EndChild();
        
        // Combo list of sprites in spritesheet
        if (ImGui::BeginCombo("Sprites", m_selectedSpriteName.length() ? m_selectedSpriteName.c_str() : "Select a sprite"))
        {
            for (auto& spr : m_sheet.getSprites())
            {
                bool sprSelected(false);
                ImGui::Selectable(spr.first.c_str(), &sprSelected);
                if (sprSelected)
                {
                    m_selectedSpriteName = spr.first;
                    m_previewScene.destroyEntity(m_previewSprite);
                    m_previewScene.createEntity();
                    m_previewSprite = m_previewScene.createEntity();
                    m_previewSprite.addComponent(spr.second);
                    m_previewSprite.getComponent<xy::Sprite>().setTexture(tex);
                    m_previewSprite.getComponent<xy::Sprite>().setTextureRect(spr.second.getTextureRect());
                    m_previewSprite.addComponent<xy::Transform>();
                    m_previewSprite.addComponent<xy::Drawable>();
                    m_previewSprite.addComponent<xy::SpriteAnimation>();
                }
            }
            ImGui::EndCombo();
        }
        
        // Sprite texture rect
        if (ImGui::InputInt4("Texture Rect", (int*)&texRect))
        {
            m_previewSprite.getComponent<xy::Sprite>().setTextureRect(sf::FloatRect(texRect));
        }
        
        // Sprite Colour
        ImVec4 col = m_previewSprite.getComponent<xy::Sprite>().getColour();
        if (ImGui::ColorEdit3("Colour", (float*)&col))
        {
            m_previewSprite.getComponent<xy::Sprite>().setColour(col);
        }
        
        // Animation combo list
        if (ImGui::BeginCombo("Animations", m_selectedAnimName.length() ? m_selectedAnimName.c_str() : "Select an animation"))
        {
            for (auto& anim : m_previewSprite.getComponent<xy::Sprite>().getAnimations())
            {
                bool animSelected(false);
                ImGui::Selectable(anim.id.data(), &animSelected);
                if (animSelected)
                {
                    m_selectedAnimName = anim.id.data();
                    m_previewSprite.getComponent<xy::SpriteAnimation>().play(m_sheet.getAnimationIndex(m_selectedAnimName, m_selectedSpriteName));
                    
                    // Make sure the entity is added to the system
                    //m_previewScene.getSystem<xy::SpriteAnimator>().addEntity(m_previewSprite);
                }
            }
            ImGui::EndCombo();
        }
        
        // Selected animation details
        if (m_selectedAnimName.length())
        {
            for (auto& anim : m_previewSprite.getComponent<xy::Sprite>().getAnimations())
            {
                if (std::string(anim.id.data()) == m_selectedAnimName)
                {
                    ImGui::InputFloat("Framerate", &anim.framerate);
                }
            }
        }
    }
    ImGui::EndDock();
}

void SpriteAsset::drawPreview()
{
    // draw the preview scene
    m_previewBuffer.clear();
    m_previewBuffer.draw(m_previewScene);
    m_previewBuffer.display();
    
    if (ImGui::BeginDock("Preview scene"))
    {
        ImGui::Image(m_previewBuffer);
    }
    ImGui::EndDock();
}
