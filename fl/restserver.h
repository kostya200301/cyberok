#ifndef RESTSERVER_H
#define RESTSERVER_H

#include "DomainResolver.h"

#include <string>
#include <cpprest/http_listener.h>
#include <cpprest/json.h>
#include <cpprest/http_listener.h>
#include <cpprest/json.h>
using namespace web;
using namespace web::http;
using namespace web::http::experimental::listener;


class RestServer {
public:
	static DomainResolver* dr;

public:
	RestServer();
        RestServer(DomainResolver* dr_, const string& serverREST);

private:
        static void handle_post(http_request request);
};


#endif
