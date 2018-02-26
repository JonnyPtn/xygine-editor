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
    SpriteAsset(const std::string& assetPath, xy::Scene& previewScene, xy::TextureResource& textures);
    void drawProperties() override;
    AssetType type() const override {return AssetType::Sprite;}
    
private:
    xy::SpriteSheet     m_sheet;
    sf::Texture*        m_texture;
    std::string         m_assetPath;
    std::string         m_selectedSpriteName;
    std::string         m_selectedAnimName;
    
    xy::Entity          m_previewSprite;
};
