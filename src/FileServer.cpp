/*
 * FileServer.cpp
 *
 *  Created on: Jul 30, 2020
 *      Author: mad
 */

#include <vnx/addons/FileServer.h>
#include <vnx/addons/permission_e.hxx>
#include <vnx/vnx.h>


namespace vnx {
namespace addons {

template<typename T>
void check_permission(std::shared_ptr<const HttpRequest> request, const T& perm)
{
	if(!request->session) {
		throw std::logic_error("session not set");
	}
	if(!vnx::is_allowed(request->session->vsid, perm)) {
		throw vnx::permission_denied(perm);
	}
}

void check_path(const std::string& path)
{
	if(path == ".."
		|| path.find("/..") != std::string::npos
		|| path.find("\\..") != std::string::npos)
	{
		throw std::logic_error("parent traversal not allowed");
	}
}


FileServer::FileServer(const std::string& _vnx_name)
	:	FileServerBase(_vnx_name)
{
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
	mime_type_map[".lua"] = "text/x-lua";
	mime_type_map[".sh"] = "text/x-shellscript";
	mime_type_map[".pdf"] = "application/pdf";
}

void FileServer::init()
{
	vnx::open_pipe(vnx_name, this, max_queue_ms, max_queue_size);
}

void FileServer::main()
{
	if(www_root.empty()) {
		throw std::logic_error("www_root not set");
	}
	if(www_root.back() != '/') {
		www_root.push_back('/');
	}
	log(INFO) << "Running on '" << www_root << "'";

	Super::main();
}

vnx::Buffer FileServer::read_file(const std::string& path) const
{
	check_path(path);

	vnx::File file(www_root + path);
	file.open("rb");

	vnx::Buffer content;
	const auto size = file.file_size();
	content.resize(size);
	file.in.read((char*)content.data(), size);
	return content;
}

vnx::Buffer FileServer::read_file_range(const std::string& path, const int64_t& offset, const int64_t& length) const
{
	check_path(path);

	vnx::File file(www_root + path);
	const int64_t file_size = file.file_size();

	int64_t offset_ = offset;
	if(offset_ < 0) {
		offset_ = file_size + offset_;
	}
	if(offset_ < 0) {
		throw std::logic_error("invalid offset: " + std::to_string(offset));
	}
	int64_t length_ = length;
	if(length_ < 0 || offset + length_ > file_size) {
		length_ = file_size - offset_;
	}
	if(length_ < 0) {
		throw std::logic_error("invalid length: " + std::to_string(length));
	}
	vnx::Buffer content;
	content.resize(length_);
	file.open("rb");
	file.seek_to(offset_);
	file.in.read((char*)content.data(), length_);
	return content;
}

file_info_t FileServer::get_file_info(const std::string& path) const
{
	check_path(path);

	vnx::File file(www_root + path);
	file_info_t info;
	info.name = file.get_name();
	info.mime_type = detect_mime_type(path);
	info.size = file.file_size();
	info.last_modified = file.last_write_time();
	return info;
}

std::vector<file_info_t> FileServer::read_directory(const std::string& path) const
{
	check_path(path);

	std::vector<file_info_t> files;
	vnx::Directory dir(www_root + path);
	dir.open();
	for(auto sub : dir.directories()) {
		file_info_t info;
		info.name = sub->get_path().substr(dir.get_path().size());
		info.mime_type = detect_mime_type(sub->get_path());
		info.is_directory = true;
		files.push_back(info);
	}
	for(auto file : dir.files()) {
		file_info_t info;
		info.name = file->get_path().substr(dir.get_path().size());
		info.mime_type = detect_mime_type(file->get_path());
		info.size = file->file_size();
		info.last_modified = file->last_write_time();
		files.push_back(info);
	}
	if(!show_hidden) {
		std::vector<file_info_t> tmp;
		for(const auto& file : files) {
			if(!file.name.empty() && file.name[0] != '.') {
				tmp.push_back(file);
			}
		}
		files = tmp;
	}
	std::sort(files.begin(), files.end(), [](const file_info_t& lhs, const file_info_t& rhs) -> bool {
		return lhs.name < rhs.name;
	});
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
	check_path(path);

	vnx::File file(www_root + path);
	file.open("wb");
	file.out.write(data.data(), data.size());
	file.close();
}

void FileServer::delete_file(const std::string& path)
{
	delete_file_internal(path);
}

void FileServer::delete_file_internal(const std::string& path) const
{
	if(read_only) {
		throw std::runtime_error("permission denied (read only)");
	}
	check_path(path);

	vnx::File file(www_root + path);
	file.remove();
}

std::string FileServer::detect_mime_type(const std::string& path) const
{
	if(!path.empty() && path.back() == '/') {
		return "inode/directory";
	}
	const vnx::File file(www_root + path);
	const auto iter = mime_type_map.find(file.get_extension());
	if(iter != mime_type_map.end()) {
		return iter->second;
	}
	return "application/octet-stream";
}

void FileServer::http_request_async(std::shared_ptr<const HttpRequest> request,
									const std::string& sub_path,
									const vnx::request_id_t& request_id) const
{
	auto response = HttpResponse::create();
	std::string file_path;
	if(!http_request_boilerplate(request, response, sub_path, file_path)){
		try {
			const vnx::File file(www_root + file_path);
			if(!file.exists()) {
				throw std::runtime_error("no such file");
			}
			if(!file.is_regular()) {
				throw std::runtime_error("not a regular file");
			}
			const auto file_size = file.file_size();
			if(file_size > limit_no_chunk){
				// leave payload empty and signal chunked transfer to caller
				response->is_chunked = true;
				response->total_size = file_size;
			}else{
				response->is_chunked = false;
				response->data = read_file(file_path);
			}
			response->content_type = detect_mime_type(file_path);
		}
		catch(const std::exception& ex) {
			response->status = 404;
			response->data = ex.what();
			response->error_text = ex.what();
			response->content_type = "text/plain";
		}
	}
	http_request_async_return(request_id, response);
}

void FileServer::http_request_chunk_async(std::shared_ptr<const HttpRequest> request,
							const std::string& sub_path,
							const int64_t& offset,
							const int64_t& max_bytes,
							const vnx::request_id_t& _request_id) const
{
	auto response = HttpResponse::create();
	std::string file_path;
	if(!http_request_boilerplate(request, response, sub_path, file_path)){
		response->data = read_file_range(file_path, offset, max_bytes);
		response->content_type = detect_mime_type(file_path);
		response->is_chunked = true;
		response->is_eof = response->data.size() < max_bytes;
	}
	http_request_chunk_async_return(_request_id, response);
}

bool FileServer::http_request_boilerplate(	std::shared_ptr<const HttpRequest> request,
											std::shared_ptr<HttpResponse> response,
											const std::string &sub_path,
											std::string &file_path) const
{
	bool result = true;

	file_path = sub_path;
	if(file_path.empty()) {
		file_path = "/";
	}
	check_path(file_path);

	try {
		if(request->method == "GET" || request->method == "HEAD") {
			if(file_path.back() == '/') {
				for(const auto& file_name : directory_files) {
					const vnx::File file(www_root + file_path + file_name);
					if(file.exists()) {
						file_path = file_path + file_name;
						break;
					}
				}
			}
			if(!redirect_not_found.empty() && !vnx::File(www_root + file_path).exists()) {
				file_path = redirect_not_found;
			}
			if(file_path.back() == '/') {
				check_permission(request, permission_e::READ_DIRECTORY);
				const auto list = read_directory(file_path);
				auto format = request->query_params.find("format");
				if(format != request->query_params.end() && format->second == "json") {
					response->data = vnx::to_string(list);
					response->content_type = "application/json";
				} else {
					std::ostringstream out;
					out << "<html>\n<head>\n<title>" << file_path << "</title>\n</head>\n<body>\n<table>\n";
					out << "<tr><th>Name</th><th>Type</th><th>Size</th></tr>\n";
					if(file_path != "/") {
						out << "<tr><td><a href=\"..\">..</a></td></tr>\n";
					}
					for(const auto& info : list) {
						out << "<tr><td><a href=\"" << info.name << "\">" << info.name << "</a></td>"
							<< "<td>" << info.mime_type << "</td>"
							<< "<td style=\"text-align: right;\">" << info.size << "</td></tr>\n";
					}
					out << "</table></body>\n</html>\n";
					response->data = out.str();
					response->content_type = "text/html";
				}
			} else {
				// actual work must be done by caller
				result = false;
			}
			response->status = 200;
		}
		else if(request->method == "PUT") {
			if(file_path.back() == '/') {
				throw std::logic_error("cannot write a directory");
			}
			check_permission(request, permission_e::FILE_UPLOAD);
			write_file_internal(file_path, request->payload);
			response->status = 200;
		}
		else if(request->method == "DELETE") {
			if(file_path.back() == '/') {
				throw std::logic_error("cannot delete a directory");
			}
			check_permission(request, permission_e::FILE_DELETE);
			delete_file_internal(file_path);
			response->status = 200;
		}
		else {
			throw std::runtime_error("invalid method: " + request->method);
		}
	}
	catch(const vnx::permission_denied& ex) {
		response->status = 403;
		response->data = ex.what();
		response->error_text = ex.what();
		response->content_type = "text/plain";
	}
	return result;
}


} // addons
} // vnx
