#ifndef MUSICTRAINERV3_CROSSDOMAINVALIDATOR_H
#define MUSICTRAINERV3_CROSSDOMAINVALIDATOR_H

#include "domain/rules/ValidationPipeline.h"
#include "domain/ports/ScoreRepository.h"
#include "domain/music/Score.h"
#include <string>
#include <vector>
#include <memory>

namespace music {
namespace crossdomain {

struct ValidationResult {
	std::string message;
	bool isValid;
};

class CrossDomainValidator {
public:
	CrossDomainValidator(rules::ValidationPipeline& pipeline, ports::ScoreRepository& repository);
	~CrossDomainValidator() = default;

	ValidationResult validate(const Score& score);

private:
	rules::ValidationPipeline& pipeline;
	ports::ScoreRepository& repository;
};

} // namespace crossdomain
} // namespace music

#endif // MUSICTRAINERV3_CROSSDOMAINVALIDATOR_H