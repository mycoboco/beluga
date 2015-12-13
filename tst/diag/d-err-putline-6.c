/* --input-charset=iso2022jp --exec-charset=iso2022kr --wide-exec-charset=ucs4 -Wv */
char x[] =
    "abcd生活漢字生活漢字zzz"; char y[] = "abcd生活漢字生活漢字\xzzz"

    L"\xzzzabcd生活漢字生活漢字";
