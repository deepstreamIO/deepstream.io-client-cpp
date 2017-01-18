struct Request
{
	typedef std::vector<char> Pattern;


	explicit Request(Topic, Action, const char* pattern, std::size_t size);

	Topic topic() const { return topic_; }
	Action action() const { return action_; }
	const Pattern& pattern() const { return pattern_; }


	const Topic topic_;
	const Action action_;
	const Pattern pattern_;
};
