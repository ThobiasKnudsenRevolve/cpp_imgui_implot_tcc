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
    std::string measure;
    std::string field;
    std::vector<double> time = {};  // Use double for time
    std::vector<float> value = {};
    bool is_prepared = false;
    bool updated = true;
    
    std::mutex data_mutex;

    string name;
    
public:
    
    Channel(const string& log_id, const string& measure, const string& field) : log_id(log_id), measure(measure), field(field) {}
    void AddOne(const std::pair<double, float>& new_point) {
        std::lock_guard<std::mutex> lock(data_mutex);
        if (!time.empty() && new_point.first <= time.back())
            is_prepared = false;
        time.emplace_back(new_point.first);
        value.emplace_back(new_point.second);
        updated = true;
    }
    void AddMultiple(const vector<pair<float, float>>& newPoints) {
        time.reserve(time.size() + newPoints.size());
        value.reserve(value.size() + newPoints.size());
        for (const auto& p : newPoints) {
            time.emplace_back(p.first);
            value.emplace_back(p.second);
        }
        is_prepared = false;
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

};

void plot(Channel& channel) {
    std::vector<double> plot_time;
    std::vector<float> plot_value;
    channel.GetDataForPlot(plot_time, plot_value);
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
        ImPlot::SetupAxisLimits(ImAxis_X1, min_time - time_padding, max_time + time_padding, ImPlotCond_Always);
        ImPlot::SetupAxisLimits(ImAxis_Y1, min_value - value_padding, max_value + value_padding, ImPlotCond_Always);
        ImPlot::PlotLine("vcu/102.INS.vx", plot_time_float.data(), plot_value.data(), static_cast<int>(plot_time_float.size()));
        ImPlot::EndPlot();
    }
}