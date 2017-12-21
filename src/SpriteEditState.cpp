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
#include "imgui_sfml.h"

SpriteEditState::SpriteEditState(xy::StateStack& stateStack, Context context) :
xy::State(stateStack, context)
{
}

bool SpriteEditState::handleEvent(const sf::Event &evt) {
}

void SpriteEditState::handleMessage(const xy::Message & msg)
{
    switch(msg.id)
    {
        case Messages::NEW_WORKING_FILE:
            m_path = msg.getData<std::string>();
            m_name = xy::FileSystem::getFileName(m_path);
            m_sheet.loadFromFile(m_path, m_texture);
            m_path = xy::FileSystem::getFilePath(m_path);
            break;

    }
}

bool SpriteEditState::update(float dt) {
}

void SpriteEditState::draw() {
    
    bool open(true);
    xy::Nim::begin(m_name, &open);
    
    // Show spritesheet first
    xy::Nim::text("Sheet texture:");
    auto& tex = m_texture.get(m_path + m_sheet.getTexturePath());
    ImGui::Image(tex);
    
    // Show sprite details
    std::vector<std::string> spriteNames;
    for (auto& spr : m_sheet.getSprites())
    {
        if (ImGui::CollapsingHeader(spr.first.c_str()))
        {
            sf::Sprite sprite;
            sprite.setTexture(tex);
            sprite.setTextureRect(sf::IntRect(spr.second.getTextureRect()));
            ImGui::Image(sprite);
        }
    }
    
    xy::Nim::end();
    
    if (!open)
        requestStackPop();
}

xy::StateID SpriteEditState::stateID() const {
    return States::SPRITE_EDIT;
}
