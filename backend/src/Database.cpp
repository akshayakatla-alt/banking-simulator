
#include "Database.h"
#include <cstdlib>
#include <iostream>
#include <pqxx/connection>
#include <pqxx/transaction>

Database::Database(const std::string& url) {

    const char* envUrl = std::getenv("DATABASE_URL");

    if (envUrl != nullptr && std::string(envUrl).length() > 0) {
        connUrl = envUrl;
    }
    else if (!url.empty()) {
        connUrl = url;
    }
    else {
        throw std::runtime_error(
            "DATABASE_URL is not configured"
        );
    }

    try {
        conn = std::make_unique<pqxx::connection>(connUrl);

        if (conn->is_open()) {
            std::cout
                << "[SJNA BANK] PostgreSQL Connected Successfully!"
                << std::endl;
        }
    }
    catch (const std::exception& e) {
        conn.reset();
        std::cerr << "[SJNA BANK ERROR] "
                  << e.what() << std::endl;
        std::cerr << "[SJNA BANK] Continuing in degraded mode; database features will fail gracefully until PostgreSQL is available."
                  << std::endl;
    }
}

Database::~Database() {
}

bool Database::isConnected() const {
    return conn && conn->is_open();
}

void Database::executeQuery(const std::string& query) {

    if (!isConnected()) {
        throw std::runtime_error("Database is not connected");
    }

    pqxx::work txn(*conn);
    txn.exec(query);
    txn.commit();
}

pqxx::connection& Database::getConnection() {

    if (!isConnected()) {
        throw std::runtime_error("Database is not connected");
    }

    return *conn;
}
