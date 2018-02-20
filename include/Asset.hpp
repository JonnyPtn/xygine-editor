//
//  Asset.hpp
//  xygine-editor
//
//  Created by Jonny Paton on 20/02/2018.
//

#pragma once

// Asset types
enum class AssetType
{
    Sprite,
    TileMap,
    ParticleEmitter,
    Texture,
    Sound
};


// Pure virtual base class for editor assets
class Asset
{
public:
    virtual void drawProperties() = 0;
    virtual void drawPreview() = 0;
    
    virtual AssetType type() const = 0;
};
