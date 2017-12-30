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
#include <xyginext/ecs/Scene.hpp>

#include <SFML/Graphics.hpp>

class SpriteEditState : public xy::State
{
public:
    SpriteEditState(xy::StateStack& stateStack, Context context, xy::Scene& previewScene);
    
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
    sf::Sprite                          m_previewSprite;
    std::vector<std::string>            m_spriteNames;
    std::string                         m_selectedSpriteName;
    std::string                         m_selectedAnimName;
    xy::Scene&                          m_previewScene;
    xy::Entity                          m_previewEntity;
    
    void updatePreview();
    
    struct AnimPreview
    {
        xy::Sprite::Animation   anim;
        sf::Sprite              sprite;
        float                   frametime = 0;
        int                     frame = 0;
    };
    
    // key is just spritename + animName
    std::unordered_map<std::string, AnimPreview>  m_animations;
    
    
    int        m_id;
    static int m_instanceCount;
    bool       m_initialised;
    sf::IntRect m_tabRect;
    bool        m_unsavedChanges;
};
