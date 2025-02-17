#ifndef MUSICTRAINERV3_RULE_H
#define MUSICTRAINERV3_RULE_H

#include <string>
#include <memory>
#include "../music/Score.h"

namespace music::rules {

class Rule {
public:
	virtual ~Rule() = default;
	
	// Evaluate if the rule is satisfied for the given score
	virtual bool evaluate(const Score& score) const = 0;
	
	// Evaluate using score snapshot
	virtual bool evaluate(const Score::ScoreSnapshot& snapshot) const {
		// Default implementation forwards to score-based evaluate
		Score tempScore(snapshot);
		return evaluate(tempScore);
	}
	
	// Get a description of why the rule failed (if it did)
	virtual std::string getViolationDescription() const = 0;
	
	// Get the name of the rule
	virtual std::string getName() const = 0;
	
	// Create a copy of the rule
	virtual std::unique_ptr<Rule> clone() const = 0;
};

} // namespace music::rules

#endif // MUSICTRAINERV3_RULE_H