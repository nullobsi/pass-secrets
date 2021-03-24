//
// Created by nullobsi on 2021/03/17.
//

#include <fstream>
#include <utility>
#include "PassCollection.h"
#include "DocumentHelper.h"
#include "PassItem.h"
#include "subprocess.h"
#include "nanoid/nanoid.h"

using namespace std;
using namespace rapidjson;


PassCollection::PassCollection(std::filesystem::path location_) : location(std::move(location_)), alias() {
	fstream docFile;
	docFile.open(location / "collection.json", ios::in);
	Document d = DHelper::ReadDocument(docFile);
	docFile.close();

	if (d.HasParseError()) goto err;
	if (!d.IsObject()) goto err;
	if (!d.HasMember("label") || !d["label"].IsString()) goto err;
	if (!d.HasMember("created") || !d["created"].IsUint64()) goto err;
	if (!d.HasMember("id") || !d["id"].IsString()) goto err;

	label = d["label"].GetString();
	created = d["created"].GetUint64();
	id = d["id"].GetString();
	if (d.HasMember("alias") && !d["alias"].IsNull()) {
		alias = d["alias"].GetString();
	}


	for (auto &entry : filesystem::directory_iterator(location)) {
		if (!entry.is_directory()) continue;
		const auto &itemPath = entry.path();
		if (!filesystem::exists(itemPath / "item.json")) continue;
		try {
			auto n = make_shared<PassItem>(itemPath);
			items.insert({n->getId(), move(n)});
		} catch (std::exception &e) {
			cerr << "Error parsing item at " + itemPath.generic_string() + "!" << endl;
		}

	}


	return;


err:
	throw std::runtime_error("Error parsing collection at " + location_.generic_string() + "!");
}

PassCollection::PassCollection(filesystem::path &location_,
                               std::string label_,
                               std::string id_,
                               int64_t created_,
                               std::string alias_) : label(std::move(label_)), id(std::move(id_)), created(created_),
                                                     location(location_), alias(std::move(alias_)) {

}

std::string
PassCollection::getId() {
	return id;
}

std::string
PassCollection::getLabel() {
	return label;
}

std::string
PassCollection::getAlias() {
	return alias;
}

uint64_t
PassCollection::getCreated() const {
	return created;
}

std::vector<std::shared_ptr<PassItem>>
PassCollection::getItems() {
	std::vector<std::shared_ptr<PassItem>> r;
	for (const auto &entry : items) {
		r.push_back(entry.second);
	}
	return r;
}

std::vector<std::string>
PassCollection::searchItems(const map<std::string, std::string> &attribs) {
	// TODO: maybe better way to search
	std::vector<std::string> r;
	for (const auto &entry : items) {
		auto itemAttrib = entry.second->getAttrib();
		for (const auto &attrib : attribs) {
			if (itemAttrib.count(attrib.first) && itemAttrib[attrib.first] == attrib.second) {
				r.push_back(entry.second->getId());
			}
		}
	}
	return r;
}

void
PassCollection::Delete() {
	string path = (location).lexically_relative(location.parent_path().parent_path()).generic_string();
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

std::shared_ptr<PassItem>
PassCollection::CreateItem(uint8_t *data,
                           size_t len,
                           std::map<std::string, std::string> attrib,
                           std::string itemLabel,
                           std::string type) {
	string itemId = nanoid::generate();
	string path = (location / itemId / "secret").lexically_relative(location.parent_path().parent_path()).generic_string();

	const char *command_line[] = {"/usr/bin/pass", "insert", "-mf", path.c_str(), nullptr};

	struct subprocess_s subprocess;
	int res = subprocess_create(command_line, subprocess_option_e::subprocess_option_inherit_environment, &subprocess);
	if (res != 0) {
		subprocess_destroy(&subprocess);
		throw std::runtime_error("Error while spawning pass");
	}

	auto writeIn = subprocess_stdin(&subprocess);
	auto written = fwrite(data, sizeof(uint8_t), len, writeIn);
	fclose(writeIn);

	if (written != len) {
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

	Document d;
	auto itemCreated = (uint64_t)time(nullptr);
	d.SetObject();
	d.AddMember("label", itemLabel, d.GetAllocator());
	d.AddMember("created", itemCreated, d.GetAllocator());
	d.AddMember("id", itemId, d.GetAllocator());
	d.AddMember("type", type, d.GetAllocator());
	d.AddMember("attrib", DHelper::SerializeAttrib(attrib, d.GetAllocator()), d.GetAllocator());

	fstream f;
	f.open(location / itemId / "item.json", ios::trunc | ios::out);
	DHelper::WriteDocument(d, f);
	f.close();

	return make_shared<PassItem>(std::move(location), std::move(itemLabel), itemCreated, std::move(itemId), std::move(attrib), std::move(type));
}

void
PassCollection::updateMetadata() {
	Document d;
	d.SetObject();
	d.AddMember("label", label, d.GetAllocator());
	d.AddMember("created", created, d.GetAllocator());
	d.AddMember("id", id, d.GetAllocator());
	d.AddMember("alias", alias, d.GetAllocator());

	fstream f;
	f.open(location / "collection.json", ios::trunc | ios::out);
	DHelper::WriteDocument(d, f);
	f.close();
}

void
PassCollection::setLabel(std::string n) {
	label = move(n);
}

void
PassCollection::setAlias(std::string n) {
	alias = move(n);
}
