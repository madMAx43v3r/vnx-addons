/*
 * test_http_server.cpp
 *
 *  Created on: Jul 9, 2020
 *      Author: mad
 */

#include <vnx/vnx.h>
#include <vnx/Terminal.h>
#include <vnx/addons/HttpServer.h>

using namespace vnx;


int main()
{
	vnx::init("test_http_server", 0, 0);

	vnx::Handle<Terminal> terminal = new Terminal("Terminal");
	terminal.start();

	vnx::Handle<addons::HttpServer> module = new addons::HttpServer("HttpServer");
	module->output_request = "test.http_request";
	module->output_response = "test.http_response";
	module->component_map["/test/"] = "HttpServer";
	module.start();

	vnx::wait();
}
