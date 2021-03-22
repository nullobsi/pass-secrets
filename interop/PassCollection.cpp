//
// Created by nullobsi on 2021/03/17.
//

#include <fstream>
#include <utility>
#include "PassCollection.h"
#include "DocumentHelper.h"
#include "PassItem.h"
using namespace std;
using namespace rapidjson;

// TODO: Save on changes

PassCollection::PassCollection(std::filesystem::path location_): location(std::move(location_)), alias() {
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
		const auto& itemPath = entry.path();
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
                               std::string  label_,
                               std::string  id_,
                               int64_t created_,
                               std::string alias_): label(std::move(label_)), id(std::move(id_)), created(created_), location(location_), alias(std::move(alias_)) {

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
	for (const auto& entry : items) {
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
