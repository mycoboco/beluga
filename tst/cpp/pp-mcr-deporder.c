#define concat(a, b, c, d) a ## b ## c ## d
#define concat2(a, b, c, d, e, f, g, h) a ## b ##c ## d /* comment */ \
                                        ## e ## f ## g ## h

concat(1, ., a, b)
concat(0, _, 3e, +)
concat(,,,)
concat2(0, , _, , 3e, ,, +)
concat2(, 1, .,, a, , b,)
concat2(, 1, ,., a, , b,)

