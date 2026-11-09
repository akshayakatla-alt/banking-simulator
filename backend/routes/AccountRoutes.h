#ifndef ACCOUNT_ROUTES_H
#define ACCOUNT_ROUTES_H

#include "crow.h"
#include "crow/middlewares/cors.h"
#include "Bank.h"
#include "Auth.h"

using BankApp = crow::App<crow::CORSHandler>;

void registerAccountRoutes(BankApp& app, Bank& bank, Auth& auth);

#endif
