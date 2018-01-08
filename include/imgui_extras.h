
#include "imgui.h"
#include "imgui_internal.h"
#include <vector>

namespace ImGui
{
    bool BeginTimeline(const char* str_id, float max_value);
    bool TimelineEvent(const char* str_id, std::vector<float>& values);
    void EndTimeline(float* value);
}
