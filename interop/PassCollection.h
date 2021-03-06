//
// Created by nullobsi on 2021/03/17.
//

#ifndef PASS_FDO_SECRETS_PASSCOLLECTION_H
#define PASS_FDO_SECRETS_PASSCOLLECTION_H


#include <filesystem>
#include <vector>
#include <map>

class PassItem;

class PassCollection {
public:
	explicit PassCollection(std::filesystem::path location_);

	PassCollection(std::filesystem::path &location_,
	               std::string label_,
	               std::string id_,
	               int64_t created_,
	               std::string alias_);

	std::string
	getId();

	std::string
	getLabel();

	std::string
	getAlias();

	uint64_t
	getCreated() const;

	std::vector<std::shared_ptr<PassItem>>
	getItems();

	std::vector<std::string>
	searchItems(const std::map<std::string, std::string> &attrib);

	void
	Delete();

	std::shared_ptr<PassItem>
	CreateItem(uint8_t *data, size_t len, std::map<std::string, std::string> attrib, std::string itemLabel, std::string type);

	void
	updateMetadata();

	void
	setLabel(std::string n);

	void
	setAlias(std::string n);

	void
	RemoveItem(std::string id);

private:
	std::filesystem::path location;
	std::string label;
	uint64_t created;
	std::string id;
	std::string alias;

	std::map<std::string, std::shared_ptr<PassItem>> items;
};


#endif //PASS_FDO_SECRETS_PASSCOLLECTION_H
