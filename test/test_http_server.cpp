/*
 * test_http_server.cpp
 *
 *  Created on: Jul 9, 2020
 *      Author: mad
 */

#include <vnx/vnx.h>
#include <vnx/Terminal.h>
#include <vnx/addons/HttpServer.h>
#include <vnx/addons/FileServer.h>

using namespace vnx;


int main(int argc, char** argv)
{
	vnx::init("test_http_server", argc, argv);

	vnx::Handle<Terminal> terminal = new Terminal("Terminal");
	terminal.start();

	{
		vnx::Handle<addons::FileServer> module = new vnx::addons::FileServer("FileServer");
		module->www_root = "./www/";
		module->directory_files.push_back("index.html");
		module.start_detached();
	}

	vnx::Handle<addons::HttpServerBase> module = vnx::addons::new_HttpServer("HttpServer");
	module->output_request = "test.http_request";
	module->output_response = "test.http_response";
	module->components["/test/"] = "HttpServer";
	module->components["/file/"] = "FileServer";
	module.start();

	vnx::wait();
}
