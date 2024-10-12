#include <iostream>
#include <map>
#include <vector>
#include <algorithm>
#include <utility>
#include <stdexcept>
#include <limits>
#include <unordered_set>

#include "implot.h"

using namespace std;

class Channel_Map {
private:
    string log_id;
    string measure;
    string field;
    map<float, float> data; // time, value
public:
    void add(float time, float value) {
        data[time] = value;
    }
    void remove(float time) {
        data.erase(time);
    }
    void clear() {
        data.clear();
    }
    float get(float time) {

        if (data.empty())
            throw runtime_error("No data points available for interpolation.");
        if (time <= data.begin()->first)
            return data.begin()->second;
        if (time >= data.rbegin()->first) 
            return data.rbegin()->second;

        auto upper = this->data.lower_bound(time);

        if (upper == data.end())
            throw runtime_error("Interpolation failed: upper bound not found.");
        if (upper->first == time)
            return upper->second;

        auto lower = prev(upper);

        float t1 = lower->first;
        float v1 = lower->second;
        float t2 = upper->first;
        float v2 = upper->second;

        float interpolatedValue = v1 + (v2 - v1) * (time - t1) / (t2 - t1);
        return interpolatedValue;
    }
    void print() {
        for (auto& [time, value] : data) {
            cout << time << ": " << value << endl;
        }
    }

};

class Channel {
    
private:
    
    string log_id;
    string measure;
    string field;
    vector<float> time = {};    // Separate vector for time
    vector<float> value = {};   // Separate vector for value
    bool is_prepared = false;
    bool updated = true;
    
public:
    
    string name;
    
public:
    
    Channel(const string& log_id, const string& measure, const string& field) : log_id(log_id), measure(measure), field(field) {}
    void AddOne(const pair<float, float>& new_point) {
        if (!time.empty() && new_point.first <= time.back()) 
            is_prepared = false;
        time.emplace_back(new_point.first);
        value.emplace_back(new_point.second);
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
        if (!is_prepared) {
            vector<size_t> indices(time.size());
            for (size_t i = 0; i < indices.size(); ++i)
                indices[i] = i;
            // Sort indices based on the time values
            sort(indices.begin(), indices.end(), [this](size_t a, size_t b) {
                return time[a] < time[b];
            });
            vector<float> sorted_time;
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
};

class CustomChannel {

private:

    vector<Channel>* channels_ptr = {};

    vector<float> time = {};    // Separate vector for time
    vector<float> value = {};

    float (*custom_function_ptr)(float);

public:



}

class Plotter {

private:

    string name;
    vector<Channel> channels;
    vector<CustomChannel> custom_channels;


public:
    void interpret(string script) {
        // Parse the script
        // Create channels
        // Add channels to the plotter
    }
    void plot() {
        if (ImPlot::BeginPlot(name.c_str())) {
            ImPlot::SetupAxes("time","");
            ImPlot::PlotLine("f(t)", xs1, ys1, 1001);
            ImPlot::SetNextMarkerStyle(ImPlotMarker_Circle);
            ImPlot::PlotLine("g(x)", xs2, ys2, 20,ImPlotLineFlags_Segments);
            ImPlot::EndPlot();
        }
    }
}

ax = Channel("CAN_2024-07-17(102548)","vcu","INS.ax") // Channel ax("now","vcu","INS.ax")
ay = Channel("CAN_2024-07-17(102548)","vcu","INS.ay") // Channel ay("now","vcu","INS.ax")
f(t) = (ax(t)^2 + ay(t)^2)^(1/2) // float f(float t) { return sqrt(ax.GetValue(t)^2 + ay.GetValue(t)^2); }
c = Channel2(t, f(t))

PlotLine(t, (ax(t)^2 + ay(t)^2)^(1/2), "red")