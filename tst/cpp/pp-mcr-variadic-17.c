/* -Wv -X */

#define foo(...) , ## __VA_ARGS__
#define foo(...) ,##__VA_ARGS__
#define foo(...) , ##__VA_ARGS__
#define foo(...) ,## __VA_ARGS__
