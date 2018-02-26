//
//  TextureAsset.hpp
//  xygine-editor
//
//  Created by Jonny Paton on 26/02/2018.
//

#pragma once

#include "Asset.hpp"
#include <xyginext/ecs/Scene.hpp>
#include <xyginext/resources/Resource.hpp>

class TextureAsset : public Asset
{
public:
    TextureAsset(const std::string& path,  xy::TextureResource& textures);
    void drawProperties() override;
    
    AssetType type() const override;
    
private:
    sf::Texture& m_texture;
};
