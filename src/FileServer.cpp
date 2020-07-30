/*
 * FileServer.cpp
 *
 *  Created on: Jul 30, 2020
 *      Author: mad
 */

#include <vnx/addons/FileServer.h>
#include <vnx/vnx.h>


namespace vnx {
namespace addons {

FileServer::FileServer(const std::string& _vnx_name)
	:	FileServerBase(_vnx_name)
{
}

void FileServer::init()
{
	vnx::open_pipe(vnx_name, this, UNLIMITED, 100);
}

void FileServer::main()
{
	if(www_root.empty()) {
		throw std::logic_error("www_root not set");
	}

	mime_type_map[".html"] = "text/html";
	mime_type_map[".css"] = "text/css";
	mime_type_map[".js"] = "application/javascript";
	mime_type_map[".json"] = "application/json";
	mime_type_map[".png"] = "image/png";
	mime_type_map[".jpg"] = "image/jpeg";
	mime_type_map[".jpeg"] = "image/jpeg";
	mime_type_map[".svg"] = "image/svg+xml";
	mime_type_map[".ico"] = "image/x-icon";
	mime_type_map[".txt"] = "text/plain";
	mime_type_map[".sh"] = "text/plain";
	mime_type_map[".cfg"] = "text/plain";
	mime_type_map[".md"] = "text/plain";
	mime_type_map[".h"] = "text/plain";
	mime_type_map[".hpp"] = "text/plain";
	mime_type_map[".hxx"] = "text/plain";
	mime_type_map[".c"] = "text/plain";
	mime_type_map[".cpp"] = "text/plain";
	mime_type_map[".cxx"] = "text/plain";
	mime_type_map[".vni"] = "text/plain";
	mime_type_map[".md5"] = "text/plain";
	mime_type_map[".map"] = "text/plain";
	mime_type_map[".tex"] = "text/x-tex";
	mime_type_map[".pdf"] = "application/pdf";

	Super::main();
}

vnx::Buffer FileServer::read_file(const std::string& path) const
{
	vnx::File file(www_root + path);
	file.open("rb");

	vnx::Buffer content;
	const auto size = file.file_size();
	content.resize(size);
	file.in.read((char*)content.data(), size);
	return content;
}

void FileServer::http_request_async(const std::shared_ptr<const HttpRequest>& request,
									const std::string& sub_path,
									const vnx::request_id_t& request_id) const
{
	auto response = HttpResponse::create();
	try {
		response->payload = read_file(sub_path);
		response->status = 200;
		{
			const vnx::File file(sub_path);
			const auto iter = mime_type_map.find(file.get_extension());
			if(iter != mime_type_map.end()) {
				response->content_type = iter->second;
			} else {
				response->content_type = "application/octet-stream";
			}
		}
	} catch(const std::exception& ex) {
		response->status = 404;
	}
	http_request_async_return(request_id, response);
}


} // addons
} // vnx
