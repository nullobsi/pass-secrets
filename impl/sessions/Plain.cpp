//
// Created by nullobsi on 2021/03/26.
//

#include "Plain.h"

#include <utility>
namespace Sessions {
	Plain::Plain(std::weak_ptr<SecretService> parent_,
	             sdbus::IConnection &conn,
	             std::string path) : Session(std::move(parent_), conn, std::move(path)) {

	}

	std::pair<std::vector<uint8_t>, std::vector<uint8_t>>
	Plain::encryptSecret(uint8_t *secret,
	                     size_t len) {
		return std::pair(std::vector<uint8_t>(), std::vector<uint8_t>(secret, secret + len));
	}

	sdbus::Variant
	Plain::getOutput() {
		return sdbus::Variant("");
	}
}