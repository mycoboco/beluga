/* -X */
/* from https://jacquesmattheij.com/c-preprocessor-hell */

#define REVERSE 5, 4, 3, 2, 1, 0
#define ARGN(_1, _2, _3, _4, _5, N, ...) N
#define NARG_(dummy, ...) ARGN(__VA_ARGS__)
#define NARG(...) NARG_(dummy, ##__VA_ARGS__, REVERSE)

#define SPLICE(a,b) SPLICE_1(a,b)
#define SPLICE_1(a,b) SPLICE_2(a,b)
#define SPLICE_2(a,b) a##b

#define FIELD_0(...)

#define FIELD_1(field, ...) \
  somestruct.field=field;

#define FIELD_2(field, ...) \
  somestruct.field=field; FIELD_1(__VA_ARGS__)

#define FIELD_3(field, ...) \
  somestruct.field=field; FIELD_2(__VA_ARGS__)

#define FIELD_4(field, ...) \
  somestruct.field=field; FIELD_3(__VA_ARGS__)

#define FIELD_5(field, ...) \
  somestruct.field=field; FIELD_4(__VA_ARGS__)

#define FIELDS_(N, ...) \
  SPLICE(FIELD_, N)(__VA_ARGS__)

#define FIELDS(...) \
  FIELDS_(NARG(__VA_ARGS__), ##__VA_ARGS__)

FIELDS(a, b, c)
