//
// Created by nullobsi on 2021/03/17.
//

#include "PassItem.h"

#include <utility>
#include <fstream>
#include <rapidjson/rapidjson.h>
#include "DocumentHelper.h"
#include "subprocess.h"
#include "iostream"

using namespace std;
using namespace rapidjson;

PassItem::PassItem(std::filesystem::path location_) : location(move(location_)) {
	fstream docFile;
	docFile.open(location / "item.json", ios::in);
	Document d = DHelper::ReadDocument(docFile);
	docFile.close();

	if (d.HasParseError()) goto err;
	if (!d.IsObject()) goto err;
	if (!d.HasMember("label") || !d["label"].IsString()) goto err;
	if (!d.HasMember("created") || !d["created"].IsUint64()) goto err;
	if (!d.HasMember("id") || !d["id"].IsString()) goto err;
	if (!d.HasMember("type") || !d["type"].IsString()) goto err;
	if (!d.HasMember("attrib") || !d["attrib"].IsObject()) goto err;

	label = d["label"].GetString();
	created = d["created"].GetUint64();
	id = d["id"].GetString();
	type = d["type"].GetString();
	try {
		attrib = move(DHelper::ReadAttrib(d["attrib"]));
	} catch (std::exception &e) {
		goto err;
	}


	return;


err:
	throw std::runtime_error("Error parsing item at " + location_.generic_string() + "!");
}

uint64_t
PassItem::getCreated() {
	return created;
}

std::map<std::string, std::string>
PassItem::getAttrib() {
	return attrib;
}

bool
PassItem::hasAttrib(const std::string &key,
                    const std::string &val) {
	if (attrib.count(key)) {
		if (attrib[key] == val) return true;
	}
	return false;
}

std::string
PassItem::getLabel() {
	return label;
}

std::string
PassItem::getId() {
	return id;
}

std::string
PassItem::getType() {
	return type;
}

uint8_t *
PassItem::getSecret() {
	return secret;
}

size_t
PassItem::getSecretLength() {
	return secretLength;
}


#define BUF_SIZE 1024

bool
PassItem::unlock() {
	string path = (location / "secret").lexically_relative(location.parent_path().parent_path().parent_path())
	                                   .generic_string();
	const char *command_line[] = {"/usr/bin/pass", "show", path.c_str(), nullptr};

	struct subprocess_s subprocess;
	int res = subprocess_create(command_line, subprocess_option_e::subprocess_option_inherit_environment
	                                          | subprocess_option_enable_async, &subprocess);
	if (res != 0) {
		throw std::runtime_error("Error while spawning pass");
	}

	auto *buf = static_cast<uint8_t *>(malloc(sizeof(uint8_t) * BUF_SIZE));
	size_t totalRead = 0;
	long buffers = 1;

	if (buf == nullptr) {
		subprocess_destroy(&subprocess);
		throw std::runtime_error("Error allocating read buffer");
	}
	while (true) {
		// read to buffer at proper offset and size
		uint8_t *offset = buf + totalRead;
		size_t numToRead = (BUF_SIZE * buffers) - totalRead;
		size_t r = subprocess_read_stdout(&subprocess, reinterpret_cast<char *const>(offset),
		                                  numToRead * sizeof(uint8_t));
		totalRead += r;

		// check EOF
		if (r == 0) break;

		// resize buffer if needed
		if (totalRead == (BUF_SIZE * buffers)) {
			auto *nbuf = static_cast<uint8_t *>(realloc(buf, sizeof(uint8_t) * (totalRead + BUF_SIZE)));
			if (nbuf == nullptr) {
				free(buf);
				subprocess_destroy(&subprocess);
				throw std::runtime_error("Failed allocating more buffer memory!");
			}
			buffers++;
			buf = nbuf;
		}
	}
	if (totalRead == 0) {
		free(buf);
		subprocess_destroy(&subprocess);
		throw std::runtime_error("Failed to read buffer!");
	}
	// make buffer proper size
	if (totalRead != (BUF_SIZE * buffers)) {
		auto *nbuf = static_cast<uint8_t *>(realloc(buf, sizeof(uint8_t) * totalRead));
		if (nbuf == nullptr) {
			free(buf);
			subprocess_destroy(&subprocess);
			throw std::runtime_error("Failed shrinking buffer!");
		}
		buf = nbuf;
	}

	secret = buf;
	secretLength = totalRead;
	subprocess_destroy(&subprocess);

	return true;
}

bool
PassItem::isUnlocked() {
	return secret != nullptr;
}

void
PassItem::lock() {
	if (isUnlocked()) {
		free(secret);
		secretLength = 0;
	}
}

PassItem::~PassItem() {
	lock();
}

PassItem::PassItem(std::filesystem::path location_,
                   std::string label_,
                   uint64_t created_,
                   std::string id_,
                   std::map<std::string, std::string> attrib_,
                   std::string type_) : location(move(location_)), label(move(label_)), created(created_), id(move(id_)), attrib(move(attrib_)), type(move(type_)){

}

void
PassItem::updateMetadata() {
	Document d;
	d.SetObject();
	d.AddMember("label", label, d.GetAllocator());
	d.AddMember("created", created, d.GetAllocator());
	d.AddMember("id", id, d.GetAllocator());
	d.AddMember("type", type, d.GetAllocator());
	d.AddMember("attrib", DHelper::SerializeAttrib(attrib, d.GetAllocator()), d.GetAllocator());

	fstream f;
	f.open(location / "item.json", ios::out | ios::trunc);
	DHelper::WriteDocument(d, f);
	f.close();
}

void
PassItem::setLabel(std::string n) {
	label = move(n);
}

void
PassItem::setAttrib(std::map<std::string, std::string> n) {
	attrib = move(n);
}

void
PassItem::setType(std::string n) {
	type = move(n);
}

void
PassItem::setSecret(uint8_t *data,
                    size_t n) {
	secret = data;
	secretLength = n;
	string path = (location / "secret").lexically_relative(location.parent_path().parent_path().parent_path()).generic_string();

	const char *command_line[] = {"/usr/bin/pass", "insert", "-mf", path.c_str(), nullptr};

	struct subprocess_s subprocess;
	int res = subprocess_create(command_line, subprocess_option_e::subprocess_option_inherit_environment, &subprocess);
	if (res != 0) {
		subprocess_destroy(&subprocess);
		throw std::runtime_error("Error while spawning pass");
	}

	auto writeIn = subprocess_stdin(&subprocess);
	auto written = fwrite(secret, sizeof(uint8_t), secretLength, writeIn);
	fclose(writeIn);

	if (written != secretLength) {
		subprocess_terminate(&subprocess);
		subprocess_destroy(&subprocess);
		throw std::runtime_error("Could not write to pass!");
	}

	int rtn;
	res = subprocess_join(&subprocess, &rtn);
	if (res != 0) {
		subprocess_destroy(&subprocess);
		throw std::runtime_error("Error while joining with pass!");
	}
	if (rtn != 0) {
		subprocess_destroy(&subprocess);
		throw std::runtime_error("pass returned an error while writing!");
	}
	subprocess_destroy(&subprocess);
}

void
PassItem::Delete() {
	string path = (location).lexically_relative(location.parent_path().parent_path().parent_path()).generic_string();
	const char *command_line[] = {"/usr/bin/pass", "rm", "-rf", path.c_str(), nullptr};

	struct subprocess_s subprocess;
	int res = subprocess_create(command_line, subprocess_option_e::subprocess_option_inherit_environment, &subprocess);
	if (res != 0) {
		subprocess_destroy(&subprocess);
		throw std::runtime_error("Error while spawning pass");
	}
	int rtn;
	res = subprocess_join(&subprocess, &rtn);
	if (res != 0) {
		subprocess_destroy(&subprocess);
		throw std::runtime_error("Error while joining with pass!");
	}
	if (rtn != 0) {
		subprocess_destroy(&subprocess);
		throw std::runtime_error("pass returned an error while deleting!");
	}
	subprocess_destroy(&subprocess);
}
