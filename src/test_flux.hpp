#include <iostream>
#include <curl/curl.h>
#include <string>

size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp)
{
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

int main() {
    // Set up the cURL handle
    CURL* curl;
    CURLcode res;
    
    // Initialize cURL
    curl = curl_easy_init();
    if(curl) {
        // The InfluxDB server URL (adjust with your server address and port)
        std::string url = "http://localhost:8086/api/v2/query";
        
        // The authorization token (replace with your actual token if necessary)
        std::string token = "Token your_token_here";

        // The Flux query
        std::string flux_query = R"({
            "query": "from(bucket:\"example-bucket\") |> range(start: -1h) |> filter(fn: (r) => r._measurement == \"temperature\")"
        })";

        // Set headers
        struct curl_slist* headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        headers = curl_slist_append(headers, ("Authorization: " + token).c_str());

        // The response data
        std::string response_string;

        // Set cURL options
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, flux_query.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_string);

        // Perform the request
        res = curl_easy_perform(curl);

        // Check for errors
        if (res != CURLE_OK)
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
        else
            // Output the response (JSON format)
            std::cout << "Response: " << response_string << std::endl;

        // Clean up
        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);
    }

    return 0;
}
