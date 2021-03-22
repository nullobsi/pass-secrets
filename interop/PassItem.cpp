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

// TODO: check this code
#define BUF_SIZE 1024

bool
PassItem::unlock() {
	struct subprocess_s subprocess;
	string path = (location / "secret").lexically_relative(location.parent_path().parent_path().parent_path())
	                                   .generic_string();
	cout << path << endl;
	const char *command_line[] = {"pass", "show", path.c_str(), SUBPROCESS_NULL};
	int res = subprocess_create(command_line, 0, &subprocess);
	if (res != 0) {
		throw std::runtime_error("Error while spawning pass");
	}

	FILE *f = subprocess_stdout(&subprocess);
	auto *buf = static_cast<uint8_t *>(malloc(sizeof(uint8_t) * BUF_SIZE));
	size_t totalRead = 0;
	long buffers = 1;

	if (buf == nullptr) {
		subprocess_destroy(&subprocess);
		throw std::runtime_error("Error allocating read buffer");
	}
	while (true) {
		// read to buffer at proper offset and size
		size_t r = fread(buf + totalRead, sizeof(uint8_t), (BUF_SIZE * buffers) - totalRead, f);
		totalRead += r;

		// check EOF
		if (feof(f)) break;

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