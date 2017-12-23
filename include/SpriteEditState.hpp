//
//  SpriteEditState.hpp
//  xygine-editor
//
//  Created by Jonny Paton on 20/12/2017.
//

#pragma once

#include <xyginext/core/State.hpp>
#include <xyginext/graphics/SpriteSheet.hpp>
#include <xyginext/ecs/components/SpriteAnimation.hpp>
#include <xyginext/resources/Resource.hpp>
#include <xyginext/ecs/Entity.hpp>

#include <SFML/Graphics.hpp>

class SpriteEditState : public xy::State
{
public:
    SpriteEditState(xy::StateStack& stateStack, Context context);
    
    bool handleEvent(const sf::Event &evt) override;
    
    void handleMessage(const xy::Message &) override;
    
    bool update(float dt) override;
    
    void draw() override;
    
    xy::StateID stateID() const override;
    
private:
    xy::SpriteSheet                     m_sheet;
    xy::TextureResource                 m_texture;
    std::string                         m_name;
    std::string                         m_path;
    
    struct AnimPreview
    {
        xy::Sprite::Animation   anim;
        sf::Sprite              sprite;
        float                   frametime;
        int                     frame;
    };
    
    // key is just spritename + animName
    std::unordered_map<std::string, AnimPreview>  m_animations;
    
    
    int        m_id;
    static int m_instanceCount;
    bool       m_initialised;
};