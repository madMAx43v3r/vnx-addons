/*
 * test_http_client.cpp
 *
 *  Created on: Sep 19, 2024
 *      Author: mad
 */

#include <vnx/vnx.h>
#include <vnx/Terminal.h>
#include <vnx/addons/HttpClient.h>
#include <vnx/addons/HttpClientClient.hxx>

using namespace vnx;


int main(int argc, char** argv)
{
	std::map<std::string, std::string> cmd;
	cmd["P"] = "POST";
	cmd["j"] = "json";
	cmd["t"] = "text";
	cmd["m"] = "mime";
	cmd["H"] = "header";
	cmd["o"] = "options";

	vnx::init("test_http_client", argc, argv, cmd);

	std::string url;
	std::string json;
	std::string text;
	addons::http_request_options_t options;
	bool is_post = false;

	vnx::read_config("$1", url);
	vnx::read_config("POST", is_post);
	vnx::read_config("json", json);
	vnx::read_config("text", text);
	vnx::read_config("options", options);
	vnx::read_config("mime", options.content_type);
	vnx::read_config("header", options.headers);

	vnx::Handle<Terminal> terminal = new Terminal("Terminal");
	terminal.start();

	vnx::Handle<addons::HttpClient> module = new vnx::addons::HttpClient("HttpClient");
	module->output_response = "test.http_response";
	module.start();

	addons::HttpClientClient client("HttpClient");

	std::shared_ptr<const addons::HttpResponse> res;
	try {
		if(is_post) {
			if(json.size()) {
				res = client.post_json(url, json, options);
			} else {
				res = client.post_text(url, text, options);
			}
		} else {
			res = client.get(url, options);
		}
		if(!res) {
			throw std::logic_error("!res");
		}
		vnx::PrettyPrinter printer(std::cout);
		vnx::accept(printer, res);
		std::cout << std::endl;

		if(res->content_type.find("text/") == 0) {
			std::cout << res->data.as_string();
		}
		else if(res->content_type.find("application/json") == 0) {
			vnx::Variant data;
			vnx::from_string(res->data.as_string(), data);
			vnx::accept(printer, data);
			std::cout << std::endl;
		}
	}
	catch(std::exception& ex) {
		std::cerr << ex.what();
	}

	vnx::close();
}


