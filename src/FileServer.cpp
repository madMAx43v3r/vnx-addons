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

	log(INFO) << "Running on '" << www_root << "'";

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

std::vector<file_info_t> FileServer::read_directory(const std::string& path) const
{
	std::vector<file_info_t> files;
	vnx::Directory dir(www_root + path);
	dir.open();
	for(auto sub : dir.directories()) {
		file_info_t info;
		info.name = sub->get_path().substr(dir.get_path().size());
		info.is_directory = true;
		files.push_back(info);
	}
	for(auto file : dir.files()) {
		file_info_t info;
		info.name = file->get_path().substr(dir.get_path().size());
		info.size = file->file_size();
		files.push_back(info);
	}
	return files;
}

void FileServer::write_file(const std::string& path, const vnx::Buffer& data)
{
	write_file_internal(path, data);
}

void FileServer::write_file_internal(const std::string& path, const vnx::Buffer& data) const
{
	if(read_only) {
		throw std::runtime_error("permission denied (read only)");
	}
	vnx::File file(www_root + path);
	file.open("wb");
	file.out.write(data.data(), data.size());
	file.close();
}

void FileServer::http_request_async(std::shared_ptr<const HttpRequest> request,
									const std::string& sub_path,
									const vnx::request_id_t& request_id) const
{
	auto file_path = sub_path;
	if(file_path.empty()) {
		file_path = "/";
	}
	auto response = HttpResponse::create();
	if(request->method == "GET") {
		try {
			if(file_path.back() == '/') {
				for(const auto& file_name : directory_files) {
					const vnx::File file(www_root + file_path + file_name);
					if(file.exists()) {
						file_path = file_path + file_name;
						break;
					}
				}
			}
			if(file_path.back() == '/') {
				const auto list = read_directory(file_path);
				auto format = request->query_params.find("format");
				if(format != request->query_params.end() && format->second == "json") {
					response->payload = vnx::to_string(list);
					response->content_type = "application/json";
				} else {
					std::ostringstream out;
					out << "<html>\n<head>\n<title>" << file_path << "</title>\n</head>\n<body>\n<table>\n";
					out << "<tr><th>Name</th><th>Size</th></tr>\n";
					if(file_path != "/") {
						out << "<tr><td><a href=\"..\">..</a></td></tr>\n";
					}
					for(const auto& info : list) {
						out << "<tr><td><a href=\"" << info.name << "\">" << info.name
							<< "</a></td><td style=\"text-align: right;\">" << info.size << "</td></tr>\n";
					}
					out << "</table></body>\n</html>\n";
					response->payload = out.str();
					response->content_type = "text/html";
				}
			} else {
				const vnx::File file(file_path);
				if(!redirect_not_found.empty() && !file.exists()) {
					file_path = redirect_not_found;
				}
				response->payload = read_file(file_path);
				const auto iter = mime_type_map.find(file.get_extension());
				if(iter != mime_type_map.end()) {
					response->content_type = iter->second;
				} else {
					response->content_type = "application/octet-stream";
				}
			}
			response->status = 200;
		}
		catch(const std::exception& ex) {
			response->status = 404;
		}
	} else if(request->method == "PUT") {
		try {
			if(file_path.back() == '/') {
				throw std::logic_error("cannot write a directory");
			}
			write_file_internal(file_path, request->payload);
			response->status = 200;
		}
		catch(const std::exception& ex) {
			response->status = 403;
			response->payload = ex.what();
			response->content_type = "text/plain";
		}
	} else {
		response->status = 500;
		response->payload = "invalid method: " + request->method;
		response->content_type = "text/plain";
	}
	http_request_async_return(request_id, response);
}


} // addons
} // vnx
