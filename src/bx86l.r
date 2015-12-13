/*
 *  machine description for x86-linux binding
 */


/* common non-terminals */
tt(stmt)
tt(reg)
tt(con)

/* non-terminals */
tt(acon)
tt(addr)
tt(addrg)
tt(addrj)
tt(base)
tt(cmpf)
tt(cnst)
tt(con1)
tt(con2)
tt(con3)
tt(flt)
tt(fltr)
tt(index)
tt(mem1)
tt(mem2)
tt(mem4)
tt(memf)
tt(memt)
tt(mr1)
tt(mr2)
tt(mr4)
tt(mrc)
tt(mrca)
tt(rc)
tt(rc5)

/* common rules */
rr(P(reg),  OP_INDIRI1 _ OP_1 _ OP_VREGP,  0,  NULL,  "# read register\n")
rr(P(reg),  OP_INDIRI2 _ OP_1 _ OP_VREGP,  0,  NULL,  "# read register\n")
rr(P(reg),  OP_INDIRI4 _ OP_1 _ OP_VREGP,  0,  NULL,  "# read register\n")
rr(P(reg),  OP_INDIRP4 _ OP_1 _ OP_VREGP,  0,  NULL,  "# read register\n")

rr(P(stmt),  OP_ASGNI1 _ OP_2 _ OP_VREGP _ P(reg),  0,  NULL,  "#write register\n")
rr(P(stmt),  OP_ASGNI2 _ OP_2 _ OP_VREGP _ P(reg),  0,  NULL,  "#write register\n")
rr(P(stmt),  OP_ASGNI4 _ OP_2 _ OP_VREGP _ P(reg),  0,  NULL,  "#write register\n")
rr(P(stmt),  OP_ASGNP4 _ OP_2 _ OP_VREGP _ P(reg),  0,  NULL,  "#write register\n")

rr(P(con),  OP_CNSTI1,  0,  NULL,  "%a")
rr(P(con),  OP_CNSTI2,  0,  NULL,  "%a")
rr(P(con),  OP_CNSTI4,  0,  NULL,  "%a")
rr(P(con),  OP_CNSTU4,  0,  NULL,  "%a")
rr(P(con),  OP_CNSTP4,  0,  NULL,  "%a")

rr(P(stmt),  P(reg),  0,  NULL,  "")

/* op rules */
/* CNST */
rr(P(con1),  OP_CNSTI4,  0,  con1,     "$1")
rr(P(con2),  OP_CNSTI4,  0,  con2,     "$2")
rr(P(con3),  OP_CNSTI4,  0,  con3,     "$3")
rr(P(con1),  OP_CNSTU4,  0,  con1,     "$1")
rr(P(con2),  OP_CNSTU4,  0,  con2,     "$2")
rr(P(con3),  OP_CNSTU4,  0,  con3,     "$3")
rr(P(rc5),   OP_CNSTI4,  0,  range31,  "$%a")
rr(P(rc5),   OP_CNSTU4,  0,  range31,  "$%a")

/* ARG */
rr(P(stmt),  OP_ARGF4 _ OP_1 _ P(reg),                    0,  NULL,  "subl $4,%%esp\n"
                                                                     "fstps (%%esp)\n")
rr(P(stmt),  OP_ARGF8 _ OP_1 _ P(reg),                    0,  NULL,  "subl $8,%%esp\n"
                                                                     "fstpl (%%esp)\n")
rr(P(stmt),  OP_ARGFc _ OP_1 _ P(reg),                    0,  NULL,  "subl $12,%%esp\n"
                                                                     "fstpt (%%esp)\n")
rr(P(stmt),  OP_ARGI4 _ OP_1 _ P(mrca),                   1,  NULL,  "pushl %0\n")
rr(P(stmt),  OP_ARGP4 _ OP_1 _ P(mrca),                   1,  NULL,  "pushl %0\n")
rr(P(stmt),  OP_ARGB _ OP_1 _ OP_INDIRB _ OP_1 _ P(reg),  0,  NULL,  "# ARGB\n")

/* ASGN */
rr(P(stmt),  OP_ASGNI4 _ OP_2 _
                 P(addr) _
                 OP_ADDI4 _ OP_2 _ P(mem4) _ P(con1),  0,  memop,  "incl %1\n")
rr(P(stmt),  OP_ASGNI4 _ OP_2 _
                 P(addr) _
                 OP_ADDU4 _ OP_2 _ P(mem4) _ P(con1),  0,  memop,  "incl %1\n")
rr(P(stmt),  OP_ASGNP4 _ OP_2 _
                 P(addr) _
                 OP_ADDP4 _ OP_2 _ P(mem4) _ P(con1),  0,  memop,  "incl %1\n")
rr(P(stmt),  OP_ASGNI4 _ OP_2 _
                 P(addr) _
                 OP_SUBI4 _ OP_2 _ P(mem4) _ P(con1),  0,  memop,  "decl %1\n")
rr(P(stmt),  OP_ASGNI4 _ OP_2 _
                 P(addr) _
                 OP_SUBU4 _ OP_2 _ P(mem4) _ P(con1),  0,  memop,  "decl %1\n")
rr(P(stmt),  OP_ASGNP4 _ OP_2 _
                 P(addr) _
                 OP_SUBP4 _ OP_2 _ P(mem4) _ P(con1),  0,  memop,  "decl %1\n")

rr(P(stmt),  OP_ASGNI4 _ OP_2 _
                 P(addr) _
                 OP_ADDI4 _ OP_2 _ P(mem4) _ P(rc),  0,  memop,  "addl %2,%1\n")
rr(P(stmt),  OP_ASGNI4 _ OP_2 _
                 P(addr) _
                 OP_ADDU4 _ OP_2 _ P(mem4) _ P(rc),  0,  memop,  "addl %2,%1\n")
rr(P(stmt),  OP_ASGNI4 _ OP_2 _
                 P(addr) _
                 OP_SUBI4 _ OP_2 _ P(mem4) _ P(rc),  0,  memop,  "subl %2,%1\n")
rr(P(stmt),  OP_ASGNI4 _ OP_2 _
                 P(addr) _
                 OP_SUBU4 _ OP_2 _ P(mem4) _ P(rc),  0,  memop,  "subl %2,%1\n")

rr(P(stmt),  OP_ASGNI4 _ OP_2 _
                 P(addr) _
                 OP_BANDU4 _ OP_2 _ P(mem4) _ P(rc),  0,  memop,  "andl %2,%1\n")
rr(P(stmt),  OP_ASGNI4 _ OP_2 _
                 P(addr) _
                 OP_BORU4 _ OP_2 _ P(mem4) _ P(rc),   0,  memop,  "orl %2,%1\n")
rr(P(stmt),  OP_ASGNI4 _ OP_2 _
                 P(addr) _
                 OP_BXORU4 _ OP_2 _ P(mem4) _ P(rc),  0,  memop,  "xorl %2,%1\n")
rr(P(stmt),  OP_ASGNI4 _ OP_2 _
                 P(addr) _
                 OP_BCOMU4 _ OP_1 _ P(mem4),          0,  memop,  "notl %1\n")
rr(P(stmt),  OP_ASGNI4 _ OP_2 _
                 P(addr) _
                 OP_NEGI4 _ OP_1 _ P(mem4),           0,  memop,  "negl %1\n")

rr(P(stmt),  OP_ASGNI4 _ OP_2 _
                 P(addr) _
                 OP_LSHI4 _ OP_2 _ P(mem4) _ P(rc5),  0,  memop,  "sall %2,%1\n")
rr(P(stmt),  OP_ASGNI4 _ OP_2 _
                 P(addr) _
                 OP_LSHU4 _ OP_2 _ P(mem4) _ P(rc5),  0,  memop,  "shll %2,%1\n")
rr(P(stmt),  OP_ASGNI4 _ OP_2 _
                 P(addr) _
                 OP_RSHI4 _ OP_2 _ P(mem4) _ P(rc5),  0,  memop,  "sarl %2,%1\n")
rr(P(stmt),  OP_ASGNI4 _ OP_2 _
                 P(addr) _
                 OP_RSHU4 _ OP_2 _ P(mem4) _ P(rc5),  0,  memop,  "shrl %2,%1\n")

rr(P(stmt),  OP_ASGNF4 _ OP_2 _
                 P(addr) _
                 OP_CVFFc4 _ OP_1 _ P(reg),   7,  NULL,  "fstps %0\n")
rr(P(stmt),  OP_ASGNF8 _ OP_2 _
                 P(addr) _
                 OP_CVFFc8 _ OP_1 _ P(reg),   7,  NULL,  "fstpl %0\n")

rr(P(stmt),  OP_ASGNF4 _ OP_2 _ P(addr) _ P(reg),  7,  NULL,  "fstps %0\n")
rr(P(stmt),  OP_ASGNF8 _ OP_2 _ P(addr) _ P(reg),  7,  NULL,  "fstpl %0\n")
rr(P(stmt),  OP_ASGNFc _ OP_2 _ P(addr) _ P(reg),  7,  NULL,  "fstpt %0\n")
rr(P(stmt),  OP_ASGNI1 _ OP_2 _ P(addr) _ P(rc),   1,  NULL,  "movb %1,%0\n")
rr(P(stmt),  OP_ASGNI2 _ OP_2 _ P(addr) _ P(rc),   1,  NULL,  "movw %1,%0\n")
rr(P(stmt),  OP_ASGNI4 _ OP_2 _ P(addr) _ P(rc),   1,  NULL,  "movl %1,%0\n")
rr(P(stmt),  OP_ASGNP4 _ OP_2 _ P(addr) _ P(rc),   1,  NULL,  "movl %1,%0\n")
rr(P(stmt),  OP_ASGNB _ OP_2 _
                 P(reg) _
                 OP_INDIRB _ OP_1 _ P(reg),        0,  NULL,  "movl $%a,%%ecx\n"
                                                              "rep\n"
                                                              "movsb\n")

/* INDIR */
rr(P(memf),  OP_INDIRF4 _ OP_1 _ P(addr),  0,  NULL,  "s %0")
rr(P(memf),  OP_INDIRF8 _ OP_1 _ P(addr),  0,  NULL,  "l %0")
rr(P(memt),  OP_INDIRFc _ OP_1 _ P(addr),  0,  NULL,  "t %0")
rr(P(mem1),  OP_INDIRI1 _ OP_1 _ P(addr),  0,  NULL,  "%0")
rr(P(mem2),  OP_INDIRI2 _ OP_1 _ P(addr),  0,  NULL,  "%0")
rr(P(mem4),  OP_INDIRI4 _ OP_1 _ P(addr),  0,  NULL,  "%0")
rr(P(mem4),  OP_INDIRP4 _ OP_1 _ P(addr),  0,  NULL,  "%0")

/* CVFF */
rr(P(reg),   OP_CVFF4c _ OP_1 _ P(reg),  0,  NULL,  "# CVFF4c\n")
rr(P(reg),   OP_CVFF8c _ OP_1 _ P(reg),  0,  NULL,  "# CVFF8c\n")

rr(P(memf),  OP_CVFF4c _ OP_1 _
                 OP_INDIRF4 _ OP_1 _ P(addr),  0,  NULL,  "s %0")
rr(P(memf),  OP_CVFF8c _ OP_1 _
                 OP_INDIRF8 _ OP_1 _ P(addr),  0,  NULL,  "l %0")

rr(P(reg),  OP_CVFFc4 _ OP_1 _ P(reg),  12,  NULL,  "subl $4,%%esp\n"
                                                    "fstps (%%esp)\n"
                                                    "flds (%%esp)\n"
                                                    "addl $4,%%esp\n")
rr(P(reg),  OP_CVFFc8 _ OP_1 _ P(reg),  12,  NULL,  "subl $8,%%esp\n"
                                                    "fstpl (%%esp)\n"
                                                    "fldl (%%esp)\n"
                                                    "addl $8,%%esp\n")

/* CVFI */
rr(P(reg),  OP_CVFIc4 _ OP_1 _ P(reg),  31,  NULL,  "subl $8,%%esp\n"
                                                    "fnstcw 4(%%esp)\n"
                                                    "movl 4(%%esp),%%edx\n"
                                                    "movb $12,%%dh\n"
                                                    "movl %%edx,0(%%esp)\n"
                                                    "fldcw 0(%%esp)\n"
                                                    "fistpl 0(%%esp)\n"
                                                    "popl %R\n"
                                                    "fldcw 0(%%esp)\n"
                                                    "addl $4,%%esp\n")

/* CVIF */
rr(P(reg),  OP_CVIF4c _ OP_1 _
                OP_INDIRI4 _ OP_1 _ P(addr),  10,  NULL,  "fildl %0\n")
rr(P(reg),  OP_CVIF4c _ OP_1 _ P(reg),        12,  NULL,  "pushl %0\n"
                                                          "fildl (%%esp)\n"
                                                          "addl $4,%%esp\n")

/* CVII */
rr(P(reg),  OP_CVII14 _ OP_1 _
                OP_INDIRI1 _ OP_1 _ P(addr),  3,  NULL,  "movsbl %0,%R\n")
rr(P(reg),  OP_CVII24 _ OP_1 _
                OP_INDIRI2 _ OP_1 _ P(addr),  3,  NULL,  "movswl %0,%R\n")
rr(P(reg),  OP_CVII14 _ OP_1 _ P(reg),        3,  NULL,  "# extend\n")
rr(P(reg),  OP_CVII24 _ OP_1 _ P(reg),        3,  NULL,  "# extend\n")

rr(P(reg),  OP_CVII41 _ OP_1 _ P(reg),  1,  NULL,  "# truncate\n")
rr(P(reg),  OP_CVII42 _ OP_1 _ P(reg),  1,  NULL,  "# truncate\n")

/* CVIU */
rr(P(reg),  OP_CVIU14 _ OP_1 _
                OP_INDIRI1 _ OP_1 _ P(addr),  3,  NULL,          "movzbl %0,%R\n")
rr(P(reg),  OP_CVIU24 _ OP_1 _
                OP_INDIRI2 _ OP_1 _ P(addr),  3,  NULL,          "movzwl %0,%R\n")
rr(P(reg),  OP_CVIU14 _ OP_1 _ P(reg),        3,  NULL,          "# extend\n")
rr(P(reg),  OP_CVIU24 _ OP_1 _ P(reg),        3,  NULL,          "# extend\n")
rr(P(reg),  OP_CVIU44 _ OP_1 _ P(reg),        0,  gen_notarget,  "%0")
rr(P(reg),  OP_CVIU44 _ OP_1 _ P(reg),        0,  gen_move,      "movl %0,%R\n")

/* CVUI */
rr(P(reg),  OP_CVUI41 _ OP_1 _ P(reg),  1,  NULL,          "# truncate\n")
rr(P(reg),  OP_CVUI42 _ OP_1 _ P(reg),  1,  NULL,          "# truncate\n")
rr(P(reg),  OP_CVUI44 _ OP_1 _ P(reg),  0,  gen_notarget,  "%0")
rr(P(reg),  OP_CVUI44 _ OP_1 _ P(reg),  0,  gen_move,      "movl %0,%R\n")

/* CVUU - not applicable */

/* CVUP */
rr(P(reg),  OP_CVUP44 _ OP_1 _ P(reg),  0,  gen_notarget,  "%0")
rr(P(reg),  OP_CVUP44 _ OP_1 _ P(reg),  0,  gen_move,      "movl %0,%R\n")

/* CVPU */
rr(P(reg),  OP_CVPU44 _ OP_1 _ P(reg),  0,  gen_notarget,  "%0")
rr(P(reg),  OP_CVPU44 _ OP_1 _ P(reg),  0,  gen_move,      "movl %0,%R\n")

/* NEG */
rr(P(reg),  OP_NEGF4 _ OP_1 _ P(reg),  0,  NULL,  "fchs\n")
rr(P(reg),  OP_NEGF8 _ OP_1 _ P(reg),  0,  NULL,  "fchs\n")
rr(P(reg),  OP_NEGFc _ OP_1 _ P(reg),  0,  NULL,  "fchs\n")
rr(P(reg),  OP_NEGI4 _ OP_1 _ P(reg),  2,  NULL,  "?movl %0,%R\n"
                                                  "negl %R\n")

/* CALL */
rr(P(stmt),  OP_CALLF4 _ OP_1 _ P(addrj),  0,  arg,   "call %0\n"
                                                      "addl $%a,%%esp\n"
                                                      "fstp %%st(0)\n")
rr(P(stmt),  OP_CALLF8 _ OP_1 _ P(addrj),  0,  arg,   "call %0\n"
                                                      "addl $%a,%%esp\n"
                                                      "fstp %%st(0)\n")
rr(P(stmt),  OP_CALLFc _ OP_1 _ P(addrj),  0,  arg,   "call %0\n"
                                                      "addl $%a,%%esp\n"
                                                      "fstp %%st(0)\n")
rr(P(reg),   OP_CALLF4 _ OP_1 _ P(addrj),  0,  arg,   "call %0\n"
                                                      "addl $%a,%%esp\n")
rr(P(reg),   OP_CALLF8 _ OP_1 _ P(addrj),  0,  arg,   "call %0\n"
                                                      "addl $%a,%%esp\n")
rr(P(reg),   OP_CALLFc _ OP_1 _ P(addrj),  0,  arg,   "call %0\n"
                                                      "addl $%a,%%esp\n")
rr(P(reg),   OP_CALLI4 _ OP_1 _ P(addrj),  0,  arg,   "call %0\n"
                                                      "addl $%a,%%esp\n")
rr(P(stmt),  OP_CALLV _ OP_1 _ P(addrj),   0,  arg,   "call %0\n"
                                                      "addl $%a,%%esp\n")
rr(P(stmt),  OP_CALLV _ OP_1 _ P(addrj),   0,  args,  "call %0\n"
                                                      "addl $%a-4,%%esp\n")

rr(P(stmt),  OP_CALLF4 _ OP_1 _ P(addrj),  1,  NULL,  "call %0\n"
                                                      "fstp %%st(0)\n")
rr(P(stmt),  OP_CALLF8 _ OP_1 _ P(addrj),  1,  NULL,  "call %0\n"
                                                      "fstp %%st(0)\n")
rr(P(stmt),  OP_CALLFc _ OP_1 _ P(addrj),  1,  NULL,  "call %0\n"
                                                      "fstp %%st(0)\n")
rr(P(reg),   OP_CALLF4 _ OP_1 _ P(addrj),  1,  NULL,  "call %0\n")
rr(P(reg),   OP_CALLF8 _ OP_1 _ P(addrj),  1,  NULL,  "call %0\n")
rr(P(reg),   OP_CALLFc _ OP_1 _ P(addrj),  1,  NULL,  "call %0\n")
rr(P(reg),   OP_CALLI4 _ OP_1 _ P(addrj),  1,  NULL,  "call %0\n")
rr(P(stmt),  OP_CALLV _ OP_1 _ P(addrj),   1,  NULL,  "call %0\n")

/* LOAD */
rr(P(reg),  OP_LOADFc _ OP_1 _ P(memt),  0,  NULL,      "fld%0\n")
rr(P(reg),  OP_LOADF8 _ OP_1 _ P(memf),  0,  NULL,      "fld%0\n")
rr(P(reg),  OP_LOADF4 _ OP_1 _ P(memf),  0,  NULL,      "fld%0\n")
rr(P(reg),  OP_LOADI1 _ OP_1 _ P(reg),   0,  gen_move,  "movl %0,%R\n")
rr(P(reg),  OP_LOADI2 _ OP_1 _ P(reg),   0,  gen_move,  "movl %0,%R\n")
rr(P(reg),  OP_LOADI4 _ OP_1 _ P(reg),   0,  gen_move,  "movl %0,%R\n")
rr(P(reg),  OP_LOADU4 _ OP_1 _ P(reg),   0,  gen_move,  "movl %0,%R\n")
rr(P(reg),  OP_LOADP4 _ OP_1 _ P(reg),   0,  gen_move,  "movl %0,%R\n")

/* RET */
rr(P(stmt),  OP_RETF4 _ OP_1 _ P(reg),  0,  NULL,  "# ret\n")
rr(P(stmt),  OP_RETF8 _ OP_1 _ P(reg),  0,  NULL,  "# ret\n")
rr(P(stmt),  OP_RETFc _ OP_1 _ P(reg),  0,  NULL,  "# ret\n")
rr(P(stmt),  OP_RETI4 _ OP_1 _ P(reg),  0,  NULL,  "# ret\n")

/* ADDR */
rr(P(acon),   OP_ADDRGP4,  0,  NULL,  "%a")
rr(P(base),   OP_ADDRGP4,  0,  NULL,  "%a")
rr(P(addrj),  OP_ADDRGP4,  0,  NULL,  "%a")
rr(P(base),   OP_ADDRLP4,  0,  NULL,  "%a(%%ebp)")
rr(P(base),   OP_ADDRFP4,  0,  NULL,  "%a(%%ebp)")
rr(P(addrg),  OP_ADDRGP4,  0,  NULL,  "%a")
rr(P(mrca),   OP_ADDRGP4,  0,  NULL,  "$%a")

/* ADD */
rr(P(base),  OP_ADDI4 _ OP_2 _ P(reg) _ P(acon),  0,  NULL,  "%1(%0)")
rr(P(base),  OP_ADDU4 _ OP_2 _ P(reg) _ P(acon),  0,  NULL,  "%1(%0)")
rr(P(base),  OP_ADDP4 _ OP_2 _ P(reg) _ P(acon),  0,  NULL,  "%1(%0)")

rr(P(addr),  OP_ADDI4 _ OP_2 _ P(index) _ P(reg),    0,  NULL,  "(%1,%0)")
rr(P(addr),  OP_ADDU4 _ OP_2 _ P(index) _ P(reg),    0,  NULL,  "(%1,%0)")
rr(P(addr),  OP_ADDP4 _ OP_2 _ P(index) _ P(reg),    0,  NULL,  "(%1,%0)")
rr(P(addr),  OP_ADDI4 _ OP_2 _ P(index) _ P(addrg),  0,  NULL,  "%1(,%0)")
rr(P(addr),  OP_ADDU4 _ OP_2 _ P(index) _ P(addrg),  0,  NULL,  "%1(,%0)")
rr(P(addr),  OP_ADDP4 _ OP_2 _ P(index) _ P(addrg),  0,  NULL,  "%1(,%0)")

rr(P(reg),  OP_ADDF4 _ OP_2 _ P(reg) _ P(flt),  0,  NULL,  "fadd%1\n")
rr(P(reg),  OP_ADDF8 _ OP_2 _ P(reg) _ P(flt),  0,  NULL,  "fadd%1\n")
rr(P(reg),  OP_ADDFc _ OP_2 _ P(reg) _ P(flt),  0,  NULL,  "fadd%1\n")
rr(P(reg),  OP_ADDI4 _ OP_2 _ P(reg) _ P(mrc),  1,  NULL,  "?movl %0,%R\n"
                                                           "addl %1,%R\n")
rr(P(reg),  OP_ADDU4 _ OP_2 _ P(reg) _ P(mrc),  1,  NULL,  "?movl %0,%R\n"
                                                           "addl %1,%R\n")
rr(P(reg),  OP_ADDP4 _ OP_2 _ P(reg) _ P(mrc),  1,  NULL,  "?movl %0,%R\n"
                                                           "addl %1,%R\n")

/* SUB */
rr(P(reg),  OP_SUBF4 _ OP_2 _ P(reg) _ P(fltr),  0,  NULL,  "fsub%1\n")
rr(P(reg),  OP_SUBF8 _ OP_2 _ P(reg) _ P(fltr),  0,  NULL,  "fsub%1\n")
rr(P(reg),  OP_SUBFc _ OP_2 _ P(reg) _ P(fltr),  0,  NULL,  "fsub%1\n")
rr(P(reg),  OP_SUBI4 _ OP_2 _ P(reg) _ P(mrc),   1,  NULL,  "?movl %0,%R\n"
                                                            "subl %1,%R\n")
rr(P(reg),  OP_SUBU4 _ OP_2 _ P(reg) _ P(mrc),   1,  NULL,  "?movl %0,%R\n"
                                                            "subl %1,%R\n")
rr(P(reg),  OP_SUBP4 _ OP_2 _ P(reg) _ P(mrc),   1,  NULL,  "?movl %0,%R\n"
                                                            "subl %1,%R\n")

/* LSH */
rr(P(index),  OP_LSHI4 _ OP_2 _ P(reg) _ P(con1),  0,  NULL,  "%0,2")
rr(P(index),  OP_LSHI4 _ OP_2 _ P(reg) _ P(con2),  0,  NULL,  "%0,4")
rr(P(index),  OP_LSHI4 _ OP_2 _ P(reg) _ P(con3),  0,  NULL,  "%0,8")
rr(P(index),  OP_LSHU4 _ OP_2 _ P(reg) _ P(con1),  0,  NULL,  "%0,2")
rr(P(index),  OP_LSHU4 _ OP_2 _ P(reg) _ P(con2),  0,  NULL,  "%0,4")
rr(P(index),  OP_LSHU4 _ OP_2 _ P(reg) _ P(con3),  0,  NULL,  "%0,8")
rr(P(reg),    OP_LSHI4 _ OP_2 _ P(reg) _ P(rc5),   2,  NULL,  "?movl %0,%R\n"
                                                              "sall %1,%R\n")
rr(P(reg),    OP_LSHU4 _ OP_2 _ P(reg) _ P(rc5),   2,  NULL,  "?movl %0,%R\n"
                                                              "shll %1,%R\n")

/* MOD */
rr(P(reg),  OP_MODI4 _ OP_2 _ P(reg) _ P(reg),  0,  NULL,  "cdq\n"
                                                           "idivl %1\n")
rr(P(reg),  OP_MODU4 _ OP_2 _ P(reg) _ P(reg),  0,  NULL,  "xorl %%edx,%%edx\n"
                                                           "divl %1\n")

/* RSH */
rr(P(reg),  OP_RSHI4 _ OP_2 _ P(reg) _ P(rc5),  2,  NULL,  "?movl %0,%R\n"
                                                           "sarl %1,%R\n")
rr(P(reg),  OP_RSHU4 _ OP_2 _ P(reg) _ P(rc5),  2,  NULL,  "?movl %0,%R\n"
                                                           "shrl %1,%R\n")

/* BAND */
rr(P(reg),  OP_BANDU4 _ OP_2 _ P(reg) _ P(mrc),  1,  NULL,  "?movl %0,%R\n"
                                                            "andl %1,%R\n")

/* BCOM */
rr(P(reg),  OP_BCOMU4 _ OP_1 _ P(reg),  2,  NULL,  "?movl %0,%R\n"
                                                   "notl %R\n")

/* BOR */
rr(P(reg),  OP_BORU4 _ OP_2 _ P(reg) _ P(mrc),  1,  NULL,  "?movl %0,%R\n"
                                                           "orl %1,%R\n")

/* BXOR */
rr(P(reg),  OP_BXORU4 _ OP_2 _ P(reg) _ P(mrc),  1,  NULL,  "?movl %0,%R\n"
                                                            "xorl %1,%R\n")

/* DIV */
rr(P(reg),  OP_DIVF4 _ OP_2 _ P(reg) _ P(fltr),  0,  NULL,  "fdiv%1\n")
rr(P(reg),  OP_DIVF8 _ OP_2 _ P(reg) _ P(fltr),  0,  NULL,  "fdiv%1\n")
rr(P(reg),  OP_DIVFc _ OP_2 _ P(reg) _ P(fltr),  0,  NULL,  "fdiv%1\n")
rr(P(reg),  OP_DIVI4 _ OP_2 _ P(reg) _ P(reg),   0,  NULL,  "cdq\n"
                                                            "idivl %1\n")
rr(P(reg),  OP_DIVU4 _ OP_2 _ P(reg) _ P(reg),   0,  NULL,  "xorl %%edx,%%edx\n"
                                                            "divl %1\n")

/* MUL */
rr(P(reg),  OP_MULF4 _ OP_2 _ P(reg) _ P(flt),   0,  NULL,  "fmul%1\n")
rr(P(reg),  OP_MULF8 _ OP_2 _ P(reg) _ P(flt),   0,  NULL,  "fmul%1\n")
rr(P(reg),  OP_MULFc _ OP_2 _ P(reg) _ P(flt),   0,  NULL,  "fmul%1\n")
rr(P(reg),  OP_MULI4 _ OP_2 _ P(reg) _ P(mrc),  14,  NULL,  "?movl %0,%R\n"
                                                            "imull %1,%R\n")
rr(P(reg),  OP_MULI4 _ OP_2 _ P(cnst) _ P(mr4), 13,  NULL,  "imull %0,%1,%R\n")
rr(P(reg),  OP_MULU4 _ OP_2 _ P(reg) _ P(mrc),  14,  NULL,  "?movl %0,%R\n"
                                                            "imull %1,%R\n")
rr(P(reg),  OP_MULU4 _ OP_2 _ P(cnst) _ P(mr4), 13,  NULL,  "imull %0,%1,%R\n")

/* EQ */
rr(P(stmt),  OP_EQF4 _ OP_2 _ P(cmpf) _ P(reg),  0,  NULL,  "fcomp%0\n"
                                                            "fstsw %%ax\n"
                                                            "sahf\n"
                                                            "jp 1f\n"
                                                            "je %a\n"
                                                            "1:\n")
rr(P(stmt),  OP_EQF8 _ OP_2 _ P(cmpf) _ P(reg),  0,  NULL,  "fcomp%0\n"
                                                            "fstsw %%ax\n"
                                                            "sahf\n"
                                                            "jp 1f\n"
                                                            "je %a\n"
                                                            "1:\n")
rr(P(stmt),  OP_EQFc _ OP_2 _ P(cmpf) _ P(reg),  0,  NULL,  "fcomp%0\n"
                                                            "fstsw %%ax\n"
                                                            "sahf\n"
                                                            "jp 1f\n"
                                                            "je %a\n"
                                                            "1:\n")
rr(P(stmt),  OP_EQI4 _ OP_2 _ P(mem4) _ P(rc),  5,  NULL,  "cmpl %1,%0\n"
                                                           "je %a\n")
rr(P(stmt),  OP_EQI4 _ OP_2 _ P(reg) _ P(mrc),  4,  NULL,  "cmpl %1,%0\n"
                                                           "je %a\n")

/* GE */
rr(P(stmt),  OP_GEF4 _ OP_2 _ P(cmpf) _ P(reg),  0,  NULL,  "fcomp%0\n"
                                                            "fstsw %%ax\n"
                                                            "sahf\n"
                                                            "jp %a\n"
                                                            "jbe %a\n")
rr(P(stmt),  OP_GEF8 _ OP_2 _ P(cmpf) _ P(reg),  0,  NULL,  "fcomp%0\n"
                                                            "fstsw %%ax\n"
                                                            "sahf\n"
                                                            "jp %a\n"
                                                            "jbe %a\n")
rr(P(stmt),  OP_GEFc _ OP_2 _ P(cmpf) _ P(reg),  0,  NULL,  "fcomp%0\n"
                                                            "fstsw %%ax\n"
                                                            "sahf\n"
                                                            "jp %a\n"
                                                            "jbe %a\n")
rr(P(stmt),  OP_GEI4 _ OP_2 _ P(mem4) _ P(rc),  5,  NULL,  "cmpl %1,%0\n"
                                                           "jge %a\n")
rr(P(stmt),  OP_GEI4 _ OP_2 _ P(reg) _ P(mrc),  4,  NULL,  "cmpl %1,%0\n"
                                                           "jge %a\n")
rr(P(stmt),  OP_GEU4 _ OP_2 _ P(mem4) _ P(rc),  5,  NULL,  "cmpl %1,%0\n"
                                                           "jae %a\n")
rr(P(stmt),  OP_GEU4 _ OP_2 _ P(reg) _ P(mrc),  4,  NULL,  "cmpl %1,%0\n"
                                                           "jae %a\n")

/* GT */
rr(P(stmt),  OP_GTF4 _ OP_2 _ P(cmpf) _ P(reg),  0,  NULL,  "fcomp%0\n"
                                                            "fstsw %%ax\n"
                                                            "sahf\n"
                                                            "jp %a\n"
                                                            "jb %a\n")
rr(P(stmt),  OP_GTF8 _ OP_2 _ P(cmpf) _ P(reg),  0,  NULL,  "fcomp%0\n"
                                                            "fstsw %%ax\n"
                                                            "sahf\n"
                                                            "jp %a\n"
                                                            "jb %a\n")
rr(P(stmt),  OP_GTFc _ OP_2 _ P(cmpf) _ P(reg),  0,  NULL,  "fcomp%0\n"
                                                            "fstsw %%ax\n"
                                                            "sahf\n"
                                                            "jp %a\n"
                                                            "jb %a\n")
rr(P(stmt),  OP_GTI4 _ OP_2 _ P(mem4) _ P(rc),  5,  NULL,  "cmpl %1,%0\n"
                                                           "jg %a\n")
rr(P(stmt),  OP_GTI4 _ OP_2 _ P(reg) _ P(mrc),  4,  NULL,  "cmpl %1,%0\n"
                                                           "jg %a\n")
rr(P(stmt),  OP_GTU4 _ OP_2 _ P(mem4) _ P(rc),  5,  NULL,  "cmpl %1,%0\n"
                                                           "ja %a\n")
rr(P(stmt),  OP_GTU4 _ OP_2 _ P(reg) _ P(mrc),  4,  NULL,  "cmpl %1,%0\n"
                                                           "ja %a\n")

/* LE */
rr(P(stmt),  OP_LEF4 _ OP_2 _ P(cmpf) _ P(reg),  0,  NULL,  "fcomp%0\n"
                                                            "fstsw %%ax\n"
                                                            "sahf\n"
                                                            "jp %a\n"
                                                            "jae %a\n")
rr(P(stmt),  OP_LEF8 _ OP_2 _ P(cmpf) _ P(reg),  0,  NULL,  "fcomp%0\n"
                                                            "fstsw %%ax\n"
                                                            "sahf\n"
                                                            "jp %a\n"
                                                            "jae %a\n")
rr(P(stmt),  OP_LEFc _ OP_2 _ P(cmpf) _ P(reg),  0,  NULL,  "fcomp%0\n"
                                                            "fstsw %%ax\n"
                                                            "sahf\n"
                                                            "jp %a\n"
                                                            "jae %a\n")
rr(P(stmt),  OP_LEI4 _ OP_2 _ P(mem4) _ P(rc),  5,  NULL,  "cmpl %1,%0\n"
                                                           "jle %a\n")
rr(P(stmt),  OP_LEI4 _ OP_2 _ P(reg) _ P(mrc),  4,  NULL,  "cmpl %1,%0\n"
                                                           "jle %a\n")
rr(P(stmt),  OP_LEU4 _ OP_2 _ P(mem4) _ P(rc),  5,  NULL,  "cmpl %1,%0\n"
                                                           "jbe %a\n")
rr(P(stmt),  OP_LEU4 _ OP_2 _ P(reg) _ P(mrc),  4,  NULL,  "cmpl %1,%0\n"
                                                           "jbe %a\n")

/* LT */
rr(P(stmt),  OP_LTF4 _ OP_2 _ P(cmpf) _ P(reg),  0,  NULL,  "fcomp%0\n"
                                                            "fstsw %%ax\n"
                                                            "sahf\n"
                                                            "jp %a\n"
                                                            "ja %a\n")
rr(P(stmt),  OP_LTF8 _ OP_2 _ P(cmpf) _ P(reg),  0,  NULL,  "fcomp%0\n"
                                                            "fstsw %%ax\n"
                                                            "sahf\n"
                                                            "jp %a\n"
                                                            "ja %a\n")
rr(P(stmt),  OP_LTFc _ OP_2 _ P(cmpf) _ P(reg),  0,  NULL,  "fcomp%0\n"
                                                            "fstsw %%ax\n"
                                                            "sahf\n"
                                                            "jp %a\n"
                                                            "ja %a\n")
rr(P(stmt),  OP_LTI4 _ OP_2 _ P(mem4) _ P(rc),  5,  NULL,  "cmpl %1,%0\n"
                                                           "jl %a\n")
rr(P(stmt),  OP_LTI4 _ OP_2 _ P(reg) _ P(mrc),  4,  NULL,  "cmpl %1,%0\n"
                                                           "jl %a\n")
rr(P(stmt),  OP_LTU4 _ OP_2 _ P(mem4) _ P(rc),  5,  NULL,  "cmpl %1,%0\n"
                                                           "jb %a\n")
rr(P(stmt),  OP_LTU4 _ OP_2 _ P(reg) _ P(mrc),  4,  NULL,  "cmpl %1,%0\n"
                                                           "jb %a\n")

/* NE */
rr(P(stmt),  OP_NEF4 _ OP_2 _ P(cmpf) _ P(reg),  0,  NULL,  "fcomp%0\n"
                                                            "fstsw %%ax\n"
                                                            "sahf\n"
                                                            "jp %a\n"
                                                            "jne %a\n")
rr(P(stmt),  OP_NEF8 _ OP_2 _ P(cmpf) _ P(reg),  0,  NULL,  "fcomp%0\n"
                                                            "fstsw %%ax\n"
                                                            "sahf\n"
                                                            "jp %a\n"
                                                            "jne %a\n")
rr(P(stmt),  OP_NEFc _ OP_2 _ P(cmpf) _ P(reg),  0,  NULL,  "fcomp%0\n"
                                                            "fstsw %%ax\n"
                                                            "sahf\n"
                                                            "jp %a\n"
                                                            "jne %a\n")
rr(P(stmt),  OP_NEI4 _ OP_2 _ P(mem4) _ P(rc),  5,  NULL,  "cmpl %1,%0\n"
                                                           "jne %a\n")
rr(P(stmt),  OP_NEI4 _ OP_2 _ P(reg) _ P(mrc),  4,  NULL,  "cmpl %1,%0\n"
                                                           "jne %a\n")

/* JMP */
rr(P(stmt),  OP_JMPV _ OP_1 _ P(addrj),  3,  NULL,  "jmp %0\n")

/* LABEL */
rr(P(stmt),  OP_LABELV,  0,  NULL,  "%a:\n")

/* chain rules */
rr(P(cnst),  P(con),  0,  NULL,  "$%0")

rr(P(acon),   P(con),  0,  NULL,  "%0")
rr(P(base),   P(reg),  0,  NULL,  "(%0)")
rr(P(index),  P(reg),  0,  NULL,  "%0")

rr(P(addr),  P(base),   0,  NULL,  "%0")
rr(P(addr),  P(index),  0,  NULL,  "(,%0)")

rr(P(rc),  P(reg),   0,  NULL,  "%0")
rr(P(rc),  P(cnst),  0,  NULL,  "%0")

rr(P(mr1),  P(reg),   0,  NULL,  "%0")
rr(P(mr1),  P(mem1),  0,  NULL,  "%0")
rr(P(mr2),  P(reg),   0,  NULL,  "%0")
rr(P(mr2),  P(mem2),  0,  NULL,  "%0")
rr(P(mr4),  P(reg),   0,  NULL,  "%0")
rr(P(mr4),  P(mem4),  0,  NULL,  "%0")

rr(P(mrc),   P(mem4),  0,  NULL,  "%0")
rr(P(mrc),   P(rc),    0,  NULL,  "%0")
rr(P(mrca),  P(mem4),  0,  NULL,  "%0")
rr(P(mrca),  P(rc),    0,  NULL,  "%0")

rr(P(reg),  P(addr),  1,  NULL,  "leal %0,%R\n")
rr(P(reg),  P(mr1),   1,  NULL,  "movb %0,%R\n")
rr(P(reg),  P(mr2),   1,  NULL,  "movw %0,%R\n")
rr(P(reg),  P(mr4),   1,  NULL,  "movl %0,%R\n")
rr(P(reg),  P(mem1),  1,  NULL,  "movb %0,%R\n")
rr(P(reg),  P(mem2),  1,  NULL,  "movw %0,%R\n")
rr(P(reg),  P(cnst),  1,  NULL,  "mov %0,%R\n")
rr(P(rc5),  P(reg),   0,  NULL,  "%%cl")

rr(P(reg),   P(memf),  3,  NULL,  "fld%0\n")
rr(P(reg),   P(memt),  3,  NULL,  "fld%0\n")
rr(P(flt),   P(memf),  0,  NULL,  "%0")
rr(P(flt),   P(reg),   0,  NULL,  "p %%st,%%st(1)")
rr(P(fltr),  P(memf),  0,  NULL,  "%0")
rr(P(fltr),  P(reg),   0,  NULL,  "rp %%st,%%st(1)")

rr(P(addrj),  P(reg),   2,  NULL,  "*%0")
rr(P(addrj),  P(mem4),  2,  NULL,  "*%0")

rr(P(cmpf),  P(memf),  0,  NULL,  "%0")
rr(P(cmpf),  P(reg),   0,  NULL,  "p")

#undef tt
#undef rr

/* end of bx86l.r */
