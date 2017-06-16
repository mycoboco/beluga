/* -Wv */

int addi[1LL + 1LL];
int addi[-9223372036854775806LL + 9223372036854775807LL];
int addu[1ULL + 1ULL];
int addu[18446744073709551615ULL + 2ULL];

int subi[3LL - 1LL];
int subi[9223372036854775807LL - 9223372036854775806LL];
int subu[3ULL - 1ULL];
int subu[18446744073709551615ULL - 18446744073709551614ULL];

int muli[2LL * 1LL];
int muli[9223372036854775807LL * 1LL - 9223372036854775806LL];
int mulu[2ULL * 1ULL];
int mulu[18446744073709551615ULL * 1ULL - 18446744073709551614ULL];

int divi[2LL / 1LL];
int divi[9223372036854775807LL / 9223372036854775807LL];
int divu[2ULL / 1ULL];
int divu[18446744073709551615ULL / 18446744073709551615ULL];

int modi[5LL % 3LL];
int modi[9223372036854775807LL % 9223372036854775806LL];
int modu[5ULL % 3ULL];
int modu[18446744073709551615ULL / 18446744073709551614ULL];

int band[2ULL & 0x7fffffffffffffffLL];
int band[1LL & 0x7fffffffffffffffLL];
int bor[2ULL | 0ULL];
int bor[1LL | 0LL];
int bxor[2ULL ^ 0ULL];
int bxor[1LL ^ 0LL];
int bcom1[2];
int bcom1[~9223372036854775806];
int bcom2[2];
int bcom2[~0xfffffffffffffffe];

int neg1[-2LL + 4LL];
int neg1[-9223372036854775806 + 9223372036854775807];
int neg2[-2ULL + 4ULL];
int neg2[-0xffffffffffffffff];

int shi[2LL << 10 >> 10];
int shi[(9223372036854775807 << 62 >> 62) + 2];
int shu[2ULL << 10 >> 10];
int shu[0xffffffffffffffff << 63 >> 63];
