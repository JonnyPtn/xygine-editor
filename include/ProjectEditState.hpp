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

#include "Project.hpp"

#include <SFML/Graphics.hpp>

// Asset types
enum class AssetType
{
    Sprite,
    TileMap,
    ParticleEmitter,
    Texture,
    Sound
};

class ProjectEditState : public xy::State
{
public:
    ProjectEditState(xy::StateStack& stateStack, Context context);
    
    bool handleEvent(const sf::Event &evt) override;
    
    void handleMessage(const xy::Message &) override;
    
    bool update(float dt) override;
    
    void draw() override;
    
    xy::StateID stateID() const override;
    
private:
    
    void    drawAssetBrowser();
    
    Project m_currentProject;
    
    std::string m_selectedAsset;
    AssetType   m_selectedAssetType;
    
    void                                imDrawSpritesheet();
    void                                imDrawParticleEmitter();
    void                                imDrawTexture();
    
    xy::Scene                           m_SpritePreviewScene;
    xy::Scene                           m_ParticlePreviewScene;
    
    xy::Entity                          m_spritePreviewEntity;
    xy::Entity                          m_particlePreviewEntity;
    
    xy::SpriteSheet*                    m_sheet;
    xy::ParticleEmitter*                m_emitter;
    
    xy::Entity                          m_SpriteCamEntity;
    xy::Entity                          m_ParticleCamEntity;
    
    // Preview controls
    bool                                m_draggingPreview;
    sf::Vector2i                        m_lastMousePos;
    
    xy::TextureResource                 m_textures;
    sf::RenderTexture                   m_previewBuffer;
    
    // The file currently selected by the project browser
    std::string m_selectedFile;
    
    int         m_id;
    static int  m_instanceCount;
    bool        m_initialised;
    sf::IntRect m_tabRect;
    bool        m_unsavedChanges;
};
