#include <iostream>
#include <map>
#include <vector>
#include <algorithm>
#include <utility>
#include <stdexcept>
#include <limits>
#include <unordered_set>
#include <thread>
#include <mutex>
#include <chrono>

#include "implot.h"

using namespace std;

class Channel {
    
public:
    
    std::string log_id;
    std::vector<std::string> tags = {};
    std::vector<double> time = {};  // Use double for time
    std::vector<float> value = {};
    bool is_prepared = false;
    bool updated = true;
    
    std::mutex data_mutex;

    string name;
    
public:
    
    Channel(const string& log_id, const std::vector<std::string>& tags) : log_id(log_id), tags(tags) {}
    Channel(const Channel&) = delete;
    Channel& operator=(const Channel&) = delete;
    Channel(Channel&&) = delete;
    Channel& operator=(Channel&&) = delete;
    std::string GetLogId() {
        return log_id;
    }
    std::vector<std::string> GetTags() {
        return tags;
    }
    void AddDatapoint(const std::pair<double, float>& new_point) {
        std::lock_guard<std::mutex> lock(data_mutex);
        if (!time.empty() && new_point.first <= time.back())
            is_prepared = false;
        time.emplace_back(new_point.first);
        value.emplace_back(new_point.second);
        updated = true;
    }
    void AddDatapoints(const vector<pair<float, float>>& newPoints) {
        time.reserve(time.size() + newPoints.size());
        value.reserve(value.size() + newPoints.size());
        for (const auto& p : newPoints) {
            time.emplace_back(p.first);
            value.emplace_back(p.second);
        }
        is_prepared = false;
        updated = true;
    }
    void PrepareData() {
        std::lock_guard<std::mutex> lock(data_mutex);
        if (!is_prepared) {
            vector<size_t> indices(time.size());
            for (size_t i = 0; i < indices.size(); ++i)
                indices[i] = i;
            // Sort indices based on the time values
            sort(indices.begin(), indices.end(), [this](size_t a, size_t b) {
                return time[a] < time[b];
            });
            vector<double> sorted_time;
            vector<float> sorted_value;
            sorted_time.reserve(time.size());
            sorted_value.reserve(value.size());
            for (size_t idx : indices) {
                sorted_time.emplace_back(time[idx]);
                sorted_value.emplace_back(value[idx]);
            }
            // Remove duplicates by keeping the last occurrence
            // Since sorted, duplicates are adjacent
            size_t unique_count = 0;
            for (size_t i = 1; i < sorted_time.size(); ++i) {
                if (sorted_time[i] != sorted_time[unique_count]) {
                    unique_count++;
                    sorted_time[unique_count] = sorted_time[i];
                    sorted_value[unique_count] = sorted_value[i];
                } else {
                    // Replace with the latest value
                    sorted_value[unique_count] = sorted_value[i];
                }
            }
            sorted_time.resize(unique_count + 1);
            sorted_value.resize(unique_count + 1);
            // Assign back to the member vectors
            time = move(sorted_time);
            value = move(sorted_value);
            is_prepared = true;
        }
    }
    float GetValue(float query_time) {
        if (!is_prepared) 
            PrepareData();
        if (time.empty()) 
            throw runtime_error("No data points available for interpolation.");
        if (query_time <= time.front()) 
            return value.front();
        if (query_time >= time.back()) 
            return value.back();
        // Binary search to find the upper bound
        size_t left = 0;
        size_t right = time.size();
        while (left < right) {
            size_t mid = left + (right - left) / 2;
            if (time[mid] < query_time)
                left = mid + 1;
            else
                right = mid;
        }

        if (left == time.size())
            throw runtime_error("Interpolation failed: upper bound not found.");
        if (time[left] == query_time)
            return value[left];
        
        size_t lower_idx = left - 1;
        size_t upper_idx = left;
        
        float t1 = time[lower_idx];
        float v1 = value[lower_idx];
        float t2 = time[upper_idx];
        float v2 = value[upper_idx];
        
        // Linear interpolation
        return v1 + (v2 - v1) * (query_time - t1) / (t2 - t1);
    }
    void PrintData() const {
        for (size_t i = 0; i < time.size(); ++i) {
            cout << "Time: " << time[i] << ", Value: " << value[i] << '\n';
        }
    }
    void GetDataForPlot(std::vector<double>& out_time, std::vector<float>& out_value) {
        std::lock_guard<std::mutex> lock(data_mutex);
        out_time = time;
        out_value = value;
    }
    void Plot() {
        std::vector<double> plot_time;
        std::vector<float> plot_value;
        GetDataForPlot(plot_time, plot_value);
        if (plot_time.empty() || plot_value.empty()) {
            return;
        }
        double first_time = plot_time.front();
        std::vector<float> plot_time_float(plot_time.size());
        for (size_t i = 0; i < plot_time.size(); ++i) {
            plot_time_float[i] = static_cast<float>((plot_time[i] - first_time) / 1000.0);
        }
        auto [min_time_iter, max_time_iter] = std::minmax_element(plot_time_float.begin(), plot_time_float.end());
        auto [min_value_iter, max_value_iter] = std::minmax_element(plot_value.begin(), plot_value.end());
        float min_time = (*max_time_iter - *min_time_iter)>5.0f ? *max_time_iter - 5.0f : *min_time_iter;
        float max_time = *max_time_iter;
        float min_value = *min_value_iter;
        float max_value = *max_value_iter;
        float time_range = max_time - min_time;
        float value_range = max_value - min_value;
        float time_padding = (time_range == 0) ? 1.0f : time_range * 0.05f;
        float value_padding = (value_range == 0) ? 1.0f : value_range * 0.05f;
        if (ImPlot::BeginPlot("Data from Mocking Server")) {
            ImPlot::SetupAxes("Time (seconds)", "Value");
            if (updated) {
                ImPlot::SetupAxisLimits(ImAxis_X1, min_time - time_padding, max_time + time_padding, ImPlotCond_Always);
                ImPlot::SetupAxisLimits(ImAxis_Y1, min_value - value_padding, max_value + value_padding, ImPlotCond_Always);
                updated=false;
            }
            else {
                ImPlot::SetupAxisLimits(ImAxis_X1, min_time - time_padding, max_time + time_padding);
                ImPlot::SetupAxisLimits(ImAxis_Y1, min_value - value_padding, max_value + value_padding);
            }
            ImPlot::PlotLine(tags[0].c_str(), plot_time_float.data(), plot_value.data(), static_cast<int>(plot_time_float.size()));
            ImPlot::EndPlot();
        }
    }

};

class DataManager {

public:

    std::vector<std::unique_ptr<Channel>> channels = {};

public:

    Channel* GetChannelPtr(std::string log_id, std::vector<std::string> tags) {
        std::sort(tags.begin(), tags.end());
        for (const auto& channel_ptr  : channels) {
            if (channel_ptr->GetLogId() == log_id && channel_ptr->GetTags() == tags) {
                return channel_ptr.get();
            }
        }
        return nullptr;
    }
    Channel* GetChannelPtr1(std::string log_id, std::vector<std::string> tags) {
        std::sort(tags.begin(), tags.end());
        for (const auto& channel_ptr  : channels) {
            if (channel_ptr->GetLogId() == log_id && channel_ptr->GetTags() == tags) {
                return channel_ptr.get();
            }
            if (channel_ptr->GetTags() == tags) {
                printf("found correct log_id = |%s|%s\n", log_id.c_str(), channel_ptr->GetLogId().c_str());
                printf("found correct tags = %s\n", tags[0].c_str());
            }
        }
        return nullptr;
    }
    bool AddDatapoint(std::string log_id, std::vector<std::string> tags, double time, float value) {

        //printf("%s %f %f\n", tags[0].c_str(), time, value);
        std::sort(tags.begin(), tags.end());
        Channel* channel_ptr = GetChannelPtr(log_id, tags);
        if (!channel_ptr) {
            channel_ptr = CreateNewChannel(log_id, tags);
            printf("%s ", log_id.c_str());
            for (auto tag : tags)
                printf("%s ", tag.c_str());
            printf("|\n");
        }
        if (!channel_ptr) {
            printf("ERROR: could not find ptr to newly created channel\n");
            return false;
        }
        channel_ptr->AddDatapoint({time, value});
        //printf("SUCCESS: added one data point");
        return true;
    }
    Channel* CreateNewChannel(std::string log_id, std::vector<std::string> tags) {
        std::sort(tags.begin(), tags.end());
        channels.emplace_back(std::make_unique<Channel>(log_id, tags));
        return channels.back().get();
    }
    void PrintData() {
        for (const auto& channel_ptr : channels) {
            channel_ptr->PrintData();
        }
    }
    void Plot(std::string log_id, std::vector<std::string> tags) {
        std::sort(tags.begin(), tags.end());
        Channel* channel_ptr = GetChannelPtr1(log_id, tags);
        if (channel_ptr) {
            channel_ptr->Plot();
            return;
        }
        printf("did not find channel ptr\n");
    }
    void GGplot(std::string log_id) {
        Channel* channel_ax = GetChannelPtr(log_id, {"vcu/102.INS.ax"});
        Channel* channel_ay = GetChannelPtr(log_id, {"vcu/102.INS.ay"});
        if (!channel_ax || !channel_ay) {
            printf("ERROR: could not find channel ptr\n");
            return;
        }

        std::vector<float> plot_ax;
        std::vector<double> plot_ax_time;
        std::vector<float> plot_ay;
        std::vector<double> plot_ay_time;
        channel_ax->GetDataForPlot(plot_ax_time, plot_ax);
        channel_ay->GetDataForPlot(plot_ay_time, plot_ay);

        if (plot_ax_time.size() != plot_ay_time.size()) {
            printf("ERROR: ax and ay data sizes do not match\n");
        }

        auto [min_ax_iter, max_ax_iter] = std::minmax_element(plot_ax.begin(), plot_ax.end());
        auto [min_ay_iter, max_ay_iter] = std::minmax_element(plot_ay.begin(), plot_ay.end());
        float min_time = *min_ax_iter;
        float max_time = *max_ax_iter;
        float min_value = *min_ay_iter;
        float max_value = *max_ay_iter;
        float ax_range = max_time - min_time;
        float ay_range = max_value - min_value;
        float ax_padding = (ax_range == 0) ? 1.0f : ax_range * 0.05f;
        float ay_padding = (ay_range == 0) ? 1.0f : ay_range * 0.05f;
        ImVec2 plot_size = ImGui::GetContentRegionAvail();
        if (ImPlot::BeginPlot("Data from Mocking Server", plot_size)) {
            ImPlot::SetupAxes("ax (m/s^2)", "ay (m/s^2)");
            ImPlot::SetupAxisLimits(ImAxis_X1, min_time - ax_padding, max_time + ax_padding, ImPlotCond_Always);
            ImPlot::SetupAxisLimits(ImAxis_Y1, min_value - ay_padding, max_value + ay_padding, ImPlotCond_Always);
            ImPlot::PlotScatter("gg plot", plot_ax.data(), plot_ay.data(), static_cast<int>(plot_ax.size()));
            ImPlot::EndPlot();
        }
    }
    void PlotHistogram() {
        std::vector<float> data = {};
        for (float y = 0.f; y < 100.f; y+=1.f) {
            for (float x = 0.f; x < 100.f; x+=1.f) {
                if (sqrt(pow(x-50.f,2) + pow(y-50.f,2)) <= 50.f) {
                    data.push_back(x);
                }
            }
        }
        if (ImPlot::BeginPlot("Normal Distribution Histogram", "Value", "Frequency")) {
            ImPlot::PlotHistogram("Histogram", data.data(), static_cast<int>(data.size()), 200, 1.0, ImPlotRange{0.f, 100.f}, 0);
            ImPlot::EndPlot();
        }
    }
    void PlotTable(float static_x1, float static_y1, float& dynamic_x2, float static_y2) {
        if (ImGui::Begin("Plot Window", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar)) {

            ImVec2          pos = ImVec2(static_x1, static_y1);
            ImVec2          size = ImVec2(dynamic_x2-static_x1, static_y2-static_y1);

            ImGui::SetWindowPos(pos);
            ImGui::SetWindowSize(size);

            static bool     is_resizing = false;
            static float    initial_mouse_x = 0.0f;
            static float    initial_width = 300.0f;
            //ImVec2          window_pos = ImGui::GetWindowPos();
            //ImVec2          window_size = ImGui::GetWindowSize();
            
            float           handle_width = 10.0f;
            ImVec2          handle_min = ImVec2(size.x - handle_width, 0);
            ImVec2          handle_max = ImVec2(size.x, size.y);
            ImDrawList*     draw_list = ImGui::GetWindowDrawList();
            ImVec2          abs_min = pos;
            ImVec2          abs_max = ImVec2(pos.x + size.x, pos.y + size.y);
            draw_list->AddRectFilled(
                ImVec2(abs_min.x + handle_min.x, abs_min.y + handle_min.y),
                ImVec2(abs_min.x + handle_max.x, abs_min.y + handle_max.y),
                IM_COL32(100, 100, 100, 255));
            ImVec2 mouse_pos = ImGui::GetIO().MousePos;
            bool is_hovered = ImGui::IsWindowHovered() &&
                              mouse_pos.x >= (pos.x + handle_min.x) &&
                              mouse_pos.x <= (pos.x + handle_max.x) &&
                              mouse_pos.y >= pos.y &&
                              mouse_pos.y <= (pos.y + size.y);
            if (is_hovered)
                ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
            if (is_hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left)){
                is_resizing = true;
                initial_mouse_x = mouse_pos.x;
                initial_width = size.x;
            }
            if (is_resizing) {
                if (ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
                    float delta = ImGui::GetIO().MousePos.x - initial_mouse_x;
                    float new_width = initial_width + delta;
                    new_width = std::max(new_width, 100.0f); // Ensure a minimum width
                    size.x = new_width; // Update the persistent window size
                    ImGui::SetWindowSize(ImVec2(new_width, size.y));
                    dynamic_x2 = new_width + static_x1;
                }
                if (!ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
                    is_resizing = false;
                }
            }
            if (ImGui::BeginTabBar("MainTabBar")) {
                if (ImGui::BeginTabItem("Plots")) {
                    if (ImGui::TreeNodeEx("InsEstimates2.yaw_rate")) { Plot("live", { "vcu/115.InsEstimates2.yaw_rate" }); ImGui::TreePop(); }
                    if (ImGui::TreeNodeEx("INS.roll_rate_dt")) { Plot("live", { "vcu/102.INS.roll_rate_dt" }); ImGui::TreePop(); }
                    if (ImGui::TreeNodeEx("INS.roll_rate")) { Plot("live", { "vcu/102.INS.roll_rate" }); ImGui::TreePop(); }
                    if (ImGui::TreeNodeEx("GNSS.altitude")) { Plot("live", { "vcu/101.GNSS.altitude" }); ImGui::TreePop(); }
                    if (ImGui::TreeNodeEx("InsStatus.ins_status")) { Plot("live", { "vcu/117.InsStatus.ins_status" }); ImGui::TreePop(); }
                    if (ImGui::TreeNodeEx("ImuMeasurements.ax")) { Plot("live", { "vcu/116.ImuMeasurements.ax" }); ImGui::TreePop(); }
                    if (ImGui::TreeNodeEx("ImuMeasurements.ay")) { Plot("live", { "vcu/116.ImuMeasurements.ay" }); ImGui::TreePop(); }
                    if (ImGui::TreeNodeEx("ImuMeasurements.az")) { Plot("live", { "vcu/116.ImuMeasurements.az" }); ImGui::TreePop(); }
                    if (ImGui::TreeNodeEx("GGplot")) { GGplot("live"); ImGui::TreePop(); }
                    if (ImGui::TreeNodeEx("Histogram")) { PlotHistogram(); ImGui::TreePop(); }
                    ImGui::EndTabItem();
                }
                ImGui::EndTabBar();
            }
            ImGui::End();
        }
    }


};
