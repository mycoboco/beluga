/* -W --parsable */
# 1 "d-err-issue-2-a.c"
char a = "d-err-issue-2-a.c" + 1;

# 4 "d-err-issue-2-a.c"
# 1 "./d-err-issue-2-b.c" 1
# 2 "./d-err-issue-2-b.c"
# 1 "./d-err-issue-2-c.c" 1
int x = 1 + "./d-err-issue-2-c.c";
# 3 "./d-err-issue-2-b.c" 2
# 5 "./d-err-issue-2-b.c"
char "./d-err-issue-2-b.c" = 5;
# 7 "./d-err-issue-2-b.c"
char 200 = "bar";
# 5 "d-err-issue-2-a.c" 2
char *b = "d-err-issue-2-a.c"[5];

# 10 "d-err-issue-2-a.c"
# 1 "./d-err-issue-2-b.c" 1
# 5 "./d-err-issue-2-b.c"
char "./d-err-issue-2-b.c" = 5;
# 7 "./d-err-issue-2-b.c"
char 200 = "bar";
# 11 "d-err-issue-2-a.c" 2
char *c = 103["foo"];
