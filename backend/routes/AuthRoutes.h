#ifndef AUTH_ROUTES_H
#define AUTH_ROUTES_H

#include "crow.h"
#include "crow/middlewares/cors.h"
#include "Auth.h"

using BankApp = crow::App<crow::CORSHandler>;

void registerAuthRoutes(BankApp& app, Auth& auth);

#endif
