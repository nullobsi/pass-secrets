//
// Created by nullobsi on 2021/03/26.
//

#ifndef PASS_SECRETS_PLAIN_H
#define PASS_SECRETS_PLAIN_H
#include "../Session.h"

namespace Sessions {
	class Plain : public Session {
	public:
		Plain(std::weak_ptr<SecretService> parent_,
		      sdbus::IConnection &conn,
		      std::string path);

		std::pair<std::vector<uint8_t>, std::vector<uint8_t>>
		encryptSecret(uint8_t *secret,
		              size_t len) override;

		sdbus::Variant getOutput() override;
	};
}

#endif //PASS_SECRETS_PLAIN_H
