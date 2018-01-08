
#include "imgui_extras.h"
#include <vector>

static float s_max_timeline_value;

namespace ImGui
{


bool BeginTimeline(const char* str_id, float max_value)
{
    s_max_timeline_value = max_value;
    return BeginChild("str_id", {GetContentRegionAvailWidth(), GetTextLineHeightWithSpacing()});
}


static const float TIMELINE_RADIUS = 6;


bool TimelineEvent(const char* str_id, std::vector<float>& values)
{
    ImGuiWindow* win = GetCurrentWindow();
    const ImU32 inactive_color = ColorConvertFloat4ToU32(GImGui->Style.Colors[ImGuiCol_Button]);
    const ImU32 active_color = ColorConvertFloat4ToU32(GImGui->Style.Colors[ImGuiCol_ButtonHovered]);
    const ImU32 line_color = ColorConvertFloat4ToU32(GImGui->Style.Colors[ImGuiCol_ColumnActive]);
    bool changed = false;
    sf::Vector2f cursor_pos = win->DC.CursorPos;
    
    int id(0);
    for (auto& v : values)
    {
        ImVec2 pos = cursor_pos;
        pos.x += win->Size.x * v / s_max_timeline_value + TIMELINE_RADIUS;
        pos.y += TIMELINE_RADIUS;
        
        SetCursorScreenPos(sf::Vector2f(pos) - sf::Vector2f(TIMELINE_RADIUS, TIMELINE_RADIUS));
        PushID(id++);
        InvisibleButton(str_id, ImVec2(2 * TIMELINE_RADIUS, 2 * TIMELINE_RADIUS));
        if (IsItemActive() || IsItemHovered())
        {
            ImGui::SetTooltip("%f", v);
            ImVec2 a(pos.x, GetWindowContentRegionMin().y + win->Pos.y);
            ImVec2 b(pos.x, GetWindowContentRegionMax().y + win->Pos.y);
            win->DrawList->AddLine(a, b, line_color);
        }
        if (IsItemActive() && IsMouseDragging(0))
        {
            v += GetIO().MouseDelta.x / win->Size.x * s_max_timeline_value;
            changed = true;
        }
        PopID();
        win->DrawList->AddCircleFilled(
                                       pos, TIMELINE_RADIUS, IsItemActive() || IsItemHovered() ? active_color : inactive_color);
    }
    
    sf::Vector2f start = cursor_pos;
    start.x += win->Size.x * values[0] / s_max_timeline_value + 2 * TIMELINE_RADIUS;
    start.y += TIMELINE_RADIUS * 0.5f;
    sf::Vector2f end = start + sf::Vector2f(win->Size.x * (values[1] - values[0]) / s_max_timeline_value - 2 * TIMELINE_RADIUS,
                                TIMELINE_RADIUS);
    
    PushID(-1);
    SetCursorScreenPos(start);
    InvisibleButton(str_id, end - start);
    if (IsItemActive() && IsMouseDragging(0))
    {
        values[0] += GetIO().MouseDelta.x / win->Size.x * s_max_timeline_value;
        values[1] += GetIO().MouseDelta.x / win->Size.x * s_max_timeline_value;
        changed = true;
    }
    PopID();
    
    SetCursorScreenPos(cursor_pos + sf::Vector2f(0, GetTextLineHeightWithSpacing()));
    
    win->DrawList->AddRectFilled(start, end, IsItemActive() || IsItemHovered() ? active_color : inactive_color);
    
    if (values[0] > values[1])
    {
        float tmp = values[0];
        values[0] = values[1];
        values[1] = tmp;
    }
    if (values[1] > s_max_timeline_value) values[1] = s_max_timeline_value;
    if (values[0] < 0) values[0] = 0;
    return changed;
}


void EndTimeline(float* value)
{
    ImGuiWindow* win = GetCurrentWindow();
    
    ImU32 color = ColorConvertFloat4ToU32(GImGui->Style.Colors[ImGuiCol_Button]);
    ImU32 line_color = ColorConvertFloat4ToU32(GImGui->Style.Colors[ImGuiCol_Border]);
    ImU32 text_color = ColorConvertFloat4ToU32(GImGui->Style.Colors[ImGuiCol_Text]);
    float rounding = GImGui->Style.ScrollbarRounding;
    sf::Vector2f start(GetWindowContentRegionMin().x + win->Pos.x,
                 GetContentRegionMax().y - GetTextLineHeightWithSpacing() + win->Pos.y);
    sf::Vector2f end = sf::Vector2f(GetWindowContentRegionMax()) + sf::Vector2f(win->Pos);
    
    win->DrawList->AddRectFilled(start, end, color, rounding);
    
    const int LINE_COUNT = 5;
    const ImVec2 text_offset(0, GetTextLineHeightWithSpacing());
    for (int i = 0; i < LINE_COUNT; ++i)
    {
        ImVec2 a = sf::Vector2f(GetWindowContentRegionMin()) + sf::Vector2f(win->Pos) + sf::Vector2f(TIMELINE_RADIUS, 0);
        a.x += i * GetWindowContentRegionWidth() / LINE_COUNT;
        ImVec2 b = a;
        b.y = start.y;
        win->DrawList->AddLine(a, b, line_color);
        char tmp[256];
        ImFormatString(tmp, sizeof(tmp), "%.2f", i * s_max_timeline_value / LINE_COUNT);
        win->DrawList->AddText(b, text_color, tmp);
    }
    
    // Draw current time marker
    if(value)
    {
        sf::Vector2f a = { (*value / s_max_timeline_value) * (end.x - start.x), start.y};
        sf::Vector2f b = { (*value / s_max_timeline_value) * (end.x - start.x), end.y};
        win->DrawList->AddLine(a, b, ColorConvertFloat4ToU32(GImGui->Style.Colors[ImGuiCol_TextSelectedBg]));
    }
    
    EndChild();
}

}
