#pragma once

// File: src/test_mocking_server.cpp
#include "test_mocking_server.hpp"
#include <websocketpp/config/asio_no_tls_client.hpp>
#include <websocketpp/client.hpp>
#include <iostream>
#include <string>
#include "nlohmann/json.hpp"

using json = nlohmann::json;
typedef websocketpp::client<websocketpp::config::asio_client> client;
typedef websocketpp::config::asio_client::message_type::ptr message_ptr;

const std::string uri = "ws://localhost:8080/ws/datastream";

int test_mocking_server() {
    client c;

    try {
        std::cout << "Initializing WebSocket++ client..." << std::endl;

        // Disable logging (optional)
        c.clear_access_channels(websocketpp::log::alevel::all);
        c.clear_error_channels(websocketpp::log::elevel::all);

        // Initialize ASIO
        c.init_asio();

        // Set message handler
        c.set_message_handler([&c](websocketpp::connection_hdl hdl, client::message_ptr msg) {
            std::string payload = msg->get_payload();
            std::cout << "Received message: " << payload << std::endl;

            try {
                // Parse JSON message
                json dataMessage = json::parse(payload);

                // Print parsed JSON in a formatted way
                std::cout << "Parsed JSON: " << dataMessage.dump(4) << std::endl;
            }
            catch (json::parse_error& e) {
                std::cerr << "JSON Parse Error: " << e.what() << std::endl;
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
            return EXIT_FAILURE;
        }

        // Connect
        c.connect(con);

        // Run the ASIO io_service loop
        c.run();
    }
    catch (websocketpp::exception const& e) {
        std::cerr << "WebSocket++ Exception: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    catch (std::exception const& e) {
        std::cerr << "STD Exception: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
