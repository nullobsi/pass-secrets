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
#include "PassStore.h"

using namespace std;
using namespace rapidjson;
using namespace subprocess::literals;

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
	if (!d.HasMember("modified") || !d["modified"].IsUint64()) goto err;

	label = d["label"].GetString();
	created = d["created"].GetUint64();
	modified = d["modified"].GetUint64();
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
    string out;
    subprocess::command cmdline {PassStore::passLocation + " show \"" + path + "\""};
    (cmdline > out).run();

    secretLength = out.length();
    secret = static_cast<uint8_t *>(malloc(secretLength));
    memcpy(secret, out.c_str(), secretLength);

	return true;
}

bool
PassItem::isUnlocked() {
	return secret != nullptr;
}

void
PassItem::lock() {
	if (isUnlocked()) {
        memset(secret, 0, secretLength);
		free(secret);
		secret = nullptr;
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
                   std::string type_,
                   uint8_t *secret_,
                   size_t secretLen_,
                   uint64_t modified_) : location(move(location_)), label(move(label_)), created(created_), id(move(id_)), attrib(move(attrib_)), type(move(type_)), secret(secret_), secretLength(secretLen_), modified(modified_){

}

void
PassItem::updateMetadata() {
	Document d;
	d.SetObject();
	d.AddMember("label", label, d.GetAllocator());
	d.AddMember("created", created, d.GetAllocator());
	d.AddMember("modified", modified, d.GetAllocator());
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
	modified = (uint64_t)time(nullptr);
	label = move(n);
}

void
PassItem::setAttrib(std::map<std::string, std::string> n) {
	modified = (uint64_t)time(nullptr);
	attrib = move(n);
}

void
PassItem::setType(std::string n) {
	modified = (uint64_t)time(nullptr);
	type = move(n);
}

void
PassItem::setSecret(uint8_t *data,
                    size_t n) {
	modified = (uint64_t)time(nullptr);
	secret = data;
	secretLength = n;
	string path = (location / "secret").lexically_relative(location.parent_path().parent_path().parent_path()).generic_string();
    string datStr((char*)data, n);
    subprocess::command cmdline {PassStore::passLocation + " insert -mf " + path};

    (cmdline < datStr).run();
}

void
PassItem::Delete() {
	string path = (location).lexically_relative(location.parent_path().parent_path().parent_path()).generic_string();

    subprocess::command cmdline {PassStore::passLocation + " rm -rf " + path};
    cmdline.run();
}

uint64_t
PassItem::getModified() {
	return modified;
}

