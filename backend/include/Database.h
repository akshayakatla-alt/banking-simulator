
#ifndef DATABASE_H
#define DATABASE_H

#include <string>
#include <memory>
#if __has_include(<pqxx/pqxx>)
#include <pqxx/pqxx>
#else
namespace pqxx {
class connection;
}
#endif

class Database {
private:
    std::string connUrl;
    std::unique_ptr<pqxx::connection> conn;

public:
    explicit Database(
        const std::string& url =
        "postgresql://postgres@127.0.0.1:5432/sjnabank"
    );

    ~Database();

    bool isConnected() const;

    void executeQuery(const std::string& query);

    pqxx::connection& getConnection();
};

#endif
