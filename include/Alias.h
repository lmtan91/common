
#ifndef ALIAS_H_
#define ALIAS_H_

template<typename Output, typename Input>
class Alias {
public:
	static Output create( Input input ) {
		return ((UnholyUnion){ input }).unionOutput;
	}
private:
	// Union to satisfy the Strict Aliasing requirement
	typedef union {
		Input unionInput;
		Output unionOutput;
	} UnholyUnion;

	// Never instantiate me
	Alias() {}
	Alias( const Alias& );
	Alias& operator=( const Alias& );
	~Alias() {}
};

#endif // ALIAS_H_
