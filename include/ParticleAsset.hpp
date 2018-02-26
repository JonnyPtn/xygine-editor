//
//  ParticleAsset.hpp
//  xygine-editor
//
//  Created by Jonny Paton on 26/02/2018.
//

#pragma once

#include "Asset.hpp"
#include <string>
#include <xyginext/ecs/Scene.hpp>
#include <xyginext/resources/Resource.hpp>

class ParticleAsset : public Asset
{
public:
    ParticleAsset(const std::string& path, xy::Scene& previewScene, xy::TextureResource& textures);
    
    void drawProperties() override;
    
    AssetType type() const override;
    
private:
    xy::Entity m_previewEntity;
    
};
