//
//  SpriteAsset.hpp
//  xygine-editor
//
//  Created by Jonny Paton on 20/02/2018.
//

#pragma once

#include "Asset.hpp"

#include <xyginext/graphics/SpriteSheet.hpp>
#include <xyginext/resources/Resource.hpp>
#include <xyginext/ecs/Scene.hpp>

class SpriteAsset : public Asset
{
public:
    SpriteAsset(const std::string& assetPath, xy::Scene& previewScene);
    void drawProperties() override;
    void drawPreview() override;
    AssetType type() const override {return AssetType::Sprite;}
    
private:
    xy::SpriteSheet     m_sheet;
    xy::TextureResource m_textures;
    std::string         m_assetPath;
    std::string         m_selectedSpriteName;
    std::string         m_selectedAnimName;
    
    sf::RenderTexture   m_previewBuffer;
    xy::Scene&          m_previewScene;
    xy::Entity          m_previewCamera;
    xy::Entity          m_previewSprite;
};
