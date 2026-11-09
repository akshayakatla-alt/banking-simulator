#ifndef TRANSACTION_ROUTES_H
#define TRANSACTION_ROUTES_H

#include "crow.h"
#include "crow/middlewares/cors.h"
#include "Bank.h"

using BankApp = crow::App<crow::CORSHandler>;

void registerTransactionRoutes(BankApp& app, Bank& bank);

#endif
