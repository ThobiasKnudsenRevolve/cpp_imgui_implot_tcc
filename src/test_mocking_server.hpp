#pragma once
// File: src/test_mocking_server.cpp
#include <websocketpp/config/asio_no_tls_client.hpp>
#include <websocketpp/client.hpp>
#include <iostream>
#include <string>
#include "nlohmann/json.hpp"
#include <mutex>

#include "test_plotting.hpp"

using json = nlohmann::json;
typedef websocketpp::client<websocketpp::config::asio_client> client;
typedef websocketpp::config::asio_client::message_type::ptr message_ptr;

const std::string uri = "ws://localhost:8080/ws/datastream";

std::mutex channel_mutex; // Mutex for thread safety

void test_mocking_server(DataManager& data_manager) {
    client c;

    try {
        std::cout << "Initializing WebSocket++ client..." << std::endl;
        c.clear_access_channels(websocketpp::log::alevel::all);
        c.clear_error_channels(websocketpp::log::elevel::all);
        c.init_asio();
        c.set_message_handler([&c, &data_manager](websocketpp::connection_hdl hdl, client::message_ptr msg) {
            std::string payload = msg->get_payload();
            try {
                json data_message = json::parse(payload);
                if (!data_message.contains("timestamp") || !data_message["timestamp"].is_number()) {
                    printf("Invalid or missing 'timestamp' field.");
                    return;
                }
                double time = data_message["timestamp"].get<double>();
                if (data_message.contains("data") && data_message["data"].is_object()) {

                    json data_object = data_message["data"];

                    for (auto& [tag, value] : data_object.items()) {
                        if (!value.is_number())
                            continue;
                        {
                            std::lock_guard<std::mutex> lock(channel_mutex);
                            data_manager.AddDatapoint("live", {tag}, time, value);
                        }
                    }
                }
            }
            catch (json::parse_error& e) {
                std::cerr << "JSON Parse Error: " << e.what() << std::endl;
            }
            catch (std::exception& e) {
                std::cerr << "Exception: " << e.what() << std::endl;
            }
        });

        // Set open handler
        c.set_open_handler([&c](websocketpp::connection_hdl hdl) {
            std::cout << "Connection opened" << std::endl;
        });

        // Set fail handler
        c.set_fail_handler([&c](websocketpp::connection_hdl hdl) {
            client::connection_ptr con = c.get_con_from_hdl(hdl);
            std::cout << "Connection failed: " << con->get_ec().message() << std::endl;
        });

        // Set close handler
        c.set_close_handler([&c](websocketpp::connection_hdl hdl) {
            std::cout << "Connection closed" << std::endl;
        });

        // Create a connection
        websocketpp::lib::error_code ec;
        client::connection_ptr con = c.get_connection(uri, ec);
        if (ec) {
            std::cerr << "Could not create connection because: " << ec.message() << std::endl;
            return;
        }

        // Connect
        c.connect(con);

        // Run the ASIO io_service loop
        c.run();
    }
    catch (websocketpp::exception const& e) {
        std::cerr << "WebSocket++ Exception: " << e.what() << std::endl;
    }
    catch (std::exception const& e) {
        std::cerr << "STD Exception: " << e.what() << std::endl;
    }
}
