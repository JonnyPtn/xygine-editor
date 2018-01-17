// Some quick'n'dirty imgui extensions I need


#pragma once

#include "imgui-SFML.h"
#include "imgui.h"
#include <SFML/Graphics.hpp>

void ImGui::Image(const sf::RenderTexture& rendertexture, const sf::Vector2f& size,
           const sf::Color& tintColor,
           const sf::Color& borderColor)
{
    ImGui::Image((void*)rendertexture.getTexture().getNativeHandle(), size, ImVec2(0, 1), ImVec2(1, 0), tintColor, borderColor);
}
void ImGui::Image(const sf::RenderTexture& rendertexture,
           const sf::Color& tintColor,
           const sf::Color& borderColor)
{
    Image(rendertexture, static_cast<sf::Vector2f>(rendertexture.getSize()), tintColor, borderColor);
}
