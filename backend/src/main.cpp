#define CROW_MAIN
#define ASIO_STANDALONE
#define ASIO_ENABLE_OLD_SERVICES
#define _WIN32_WINNT 0x0601

#include "crow.h"
#include "crow/middlewares/cors.h"
#include "Database.h"
#include "Auth.h"
#include "Bank.h"
#include "AuthRoutes.h"
#include "AccountRoutes.h"
#include "TransactionRoutes.h"

#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <stdexcept>

// Helper to load simple KEY=VALUE from .env
void loadEnvFile(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) return;

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;
        size_t eqPos = line.find('=');
        if (eqPos != std::string::npos) {
            std::string key = line.substr(0, eqPos);
            std::string val = line.substr(eqPos + 1);
            // Trim carriage returns or spaces
            while (!val.empty() && (val.back() == '\r' || val.back() == ' ')) val.pop_back();
            while (!key.empty() && (key.back() == ' ')) key.pop_back();
            #ifdef _WIN32
            _putenv_s(key.c_str(), val.c_str());
            #else
            setenv(key.c_str(), val.c_str(), 1);
            #endif
        }
    }
}

int main() {
    loadEnvFile(".env");
    loadEnvFile("../.env");
    loadEnvFile("backend/.env");

    const char* dbUrlEnv = std::getenv("DATABASE_URL");
    std::string dbUrl = (dbUrlEnv && std::string(dbUrlEnv).length() > 0)
        ? dbUrlEnv
        : "postgresql://postgres@127.0.0.1:5432/sjnabank";

    const char* portEnv = std::getenv("PORT");
    int port = (portEnv && std::string(portEnv).length() > 0) ? std::stoi(portEnv) : 8080;

    const char* jwtSecretEnv = std::getenv("JWT_SECRET");
    if (!jwtSecretEnv || std::string(jwtSecretEnv).empty()) {
        throw std::runtime_error("JWT_SECRET is not configured");
    }
    std::string jwtSecret = jwtSecretEnv;

    std::cout << "=====================================================" << std::endl;
    std::cout << "      SJNA BANK - Transaction Simulator Backend       " << std::endl;
    std::cout << "=====================================================" << std::endl;

    // Initialize Database
    Database db(dbUrl);

    // Initialize Auth & Banking Services
    Auth auth(db, jwtSecret);
    Bank bank(db);

    // Configure Crow with CORS Middleware
    crow::App<crow::CORSHandler> app;

    auto& cors = app.get_middleware<crow::CORSHandler>();
    cors
      .global()
        .headers("Content-Type", "Authorization", "Accept", "X-Requested-With")
        .methods("POST"_method, "GET"_method, "PUT"_method, "DELETE"_method, "OPTIONS"_method)
        .origin("*");

    // Root & Health check routes
    CROW_ROUTE(app, "/")([]() {
        crow::json::wvalue res;
        res["service"] = "SJNA BANK - Online Banking Transaction Simulator";
        res["version"] = "2.0.0";
        res["status"] = "Running";
        return crow::response(200, res);
    });

    CROW_ROUTE(app, "/api/health")([&db]() {
        crow::json::wvalue res;
        res["status"] = "UP";
        res["database"] = db.isConnected() ? "Connected" : "Disconnected";
        res["system"] = "SJNA BANK Core Banking Simulator";
        return crow::response(200, res);
    });

    // Register modular route handlers
    registerAuthRoutes(app, auth);
    registerAccountRoutes(app, bank, auth);
    registerTransactionRoutes(app, bank);

    std::cout << "[SJNA BANK] Server listening on port " << port << "..." << std::endl;
    std::cout << "[SJNA BANK] REST API ready for frontend integration!" << std::endl;

    app.port(port).multithreaded().run();

    return 0;
}
