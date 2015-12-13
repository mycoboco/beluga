/*
 *  machine description for lcc's x86-test binding
 */


/* common non-terminals */
tt(stmt)
tt(reg)
tt(con)

/* non-terminals */
tt(acon)
tt(addr)
tt(addrj)
tt(base)
tt(cmpf)
tt(con1)
tt(con2)
tt(con3)
tt(flt)
tt(index)
tt(mem)
tt(memf)
tt(mr)
tt(mrc0)
tt(mrc1)
tt(mrc3)
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
rr(P(con1),  OP_CNSTI4,  0,  con1,     "1")
rr(P(con2),  OP_CNSTI4,  0,  con2,     "2")
rr(P(con3),  OP_CNSTI4,  0,  con3,     "3")
rr(P(con1),  OP_CNSTU4,  0,  con1,     "1")
rr(P(con2),  OP_CNSTU4,  0,  con2,     "2")
rr(P(con3),  OP_CNSTU4,  0,  con3,     "3")
rr(P(rc5),   OP_CNSTI4,  0,  range31,  "%a")
rr(P(rc5),   OP_CNSTU4,  0,  range31,  "%a")

/* ARG */
rr(P(stmt),  OP_ARGF4 _ OP_1 _ P(reg),                    0,  NULL,  "sub esp,4\n"
                                                                     "fstp qword ptr [esp]\n")
rr(P(stmt),  OP_ARGF8 _ OP_1 _ P(reg),                    0,  NULL,  "sub esp,8\n"
                                                                     "fstp qword ptr [esp]\n")
rr(P(stmt),  OP_ARGFc _ OP_1 _ P(reg),                    0,  NULL,  "sub esp,12\n"
                                                                     "fstp tbyte ptr [esp]\n")
rr(P(stmt),  OP_ARGI4 _ OP_1 _ P(mrc3),                   0,  NULL,  "push %0\n")
rr(P(stmt),  OP_ARGP4 _ OP_1 _ P(mrc3),                   0,  NULL,  "push %0\n")
rr(P(stmt),  OP_ARGB _ OP_1 _ OP_INDIRB _ OP_1 _ P(reg),  0,  NULL,  "sub esp,%a\n"
                                                                     "mov edi,esp\n"
                                                                     "mov ecx,%a\n"
                                                                     "rep movsb\n")

/* ASGN */
rr(P(stmt),  OP_ASGNI4 _ OP_2 _
                 P(addr) _
                 OP_ADDI4 _ OP_2 _ P(mem) _ P(con1),  0,  memop,  "inc %1\n")
rr(P(stmt),  OP_ASGNI4 _ OP_2 _
                 P(addr) _
                 OP_ADDU4 _ OP_2 _ P(mem) _ P(con1),  0,  memop,  "inc %1\n")
rr(P(stmt),  OP_ASGNP4 _ OP_2 _
                 P(addr) _
                 OP_ADDP4 _ OP_2 _ P(mem) _ P(con1),  0,  memop,  "inc %1\n")
rr(P(stmt),  OP_ASGNI4 _ OP_2 _
                 P(addr) _
                 OP_SUBI4 _ OP_2 _ P(mem) _ P(con1),  0,  memop,  "dec %1\n")
rr(P(stmt),  OP_ASGNI4 _ OP_2 _
                 P(addr) _
                 OP_SUBU4 _ OP_2 _ P(mem) _ P(con1),  0,  memop,  "dec %1\n")
rr(P(stmt),  OP_ASGNP4 _ OP_2 _
                 P(addr) _
                 OP_SUBP4 _ OP_2 _ P(mem) _ P(con1),  0,  memop,  "dec %1\n")

rr(P(stmt),  OP_ASGNI4 _ OP_2 _
                 P(addr) _
                 OP_ADDI4 _ OP_2 _ P(mem) _ P(rc),  0,  memop,  "add %1,%2\n")
rr(P(stmt),  OP_ASGNI4 _ OP_2 _
                 P(addr) _
                 OP_ADDU4 _ OP_2 _ P(mem) _ P(rc),  0,  memop,  "add %1,%2\n")
rr(P(stmt),  OP_ASGNI4 _ OP_2 _
                 P(addr) _
                 OP_SUBI4 _ OP_2 _ P(mem) _ P(rc),  0,  memop,  "sub %1,%2\n")
rr(P(stmt),  OP_ASGNI4 _ OP_2 _
                 P(addr) _
                 OP_SUBU4 _ OP_2 _ P(mem) _ P(rc),  0,  memop,  "sub %1,%2\n")

rr(P(stmt),  OP_ASGNI4 _ OP_2 _
                 P(addr) _
                 OP_BANDU4 _ OP_2 _ P(mem) _ P(rc),  0,  memop,  "and %1,%2\n")
rr(P(stmt),  OP_ASGNI4 _ OP_2 _
                 P(addr) _
                 OP_BORU4 _ OP_2 _ P(mem) _ P(rc),   0,  memop,  "or %1,%2\n")
rr(P(stmt),  OP_ASGNI4 _ OP_2 _
                 P(addr) _
                 OP_BXORU4 _ OP_2 _ P(mem) _ P(rc),  0,  memop,  "xor %1,%2\n")
rr(P(stmt),  OP_ASGNI4 _ OP_2 _
                 P(addr) _
                 OP_BCOMU4 _ OP_1 _ P(mem),          0,  memop,  "not %1\n")
rr(P(stmt),  OP_ASGNI4 _ OP_2 _
                 P(addr) _
                 OP_NEGI4 _ OP_1 _ P(mem),           0,  memop,  "neg %1\n")

rr(P(stmt),  OP_ASGNI4 _ OP_2 _
                 P(addr) _
                 OP_LSHI4 _ OP_2 _ P(mem) _ P(rc5),  0,  memop,  "sal %1,%2\n")
rr(P(stmt),  OP_ASGNI4 _ OP_2 _
                 P(addr) _
                 OP_LSHU4 _ OP_2 _ P(mem) _ P(rc5),  0,  memop,  "shl %1,%2\n")
rr(P(stmt),  OP_ASGNI4 _ OP_2 _
                 P(addr) _
                 OP_RSHI4 _ OP_2 _ P(mem) _ P(rc5),  0,  memop,  "sar %1,%2\n")
rr(P(stmt),  OP_ASGNI4 _ OP_2 _
                 P(addr) _
                 OP_RSHU4 _ OP_2 _ P(mem) _ P(rc5),  0,  memop,  "shr %1,%2\n")

rr(P(stmt),  OP_ASGNF4 _ OP_2 _
                 P(addr) _
                 OP_CVFFc4 _ OP_1 _ P(reg),   7,  NULL,  "fstp dword ptr %0\n")
rr(P(stmt),  OP_ASGNF8 _ OP_2 _
                 P(addr) _
                 OP_CVFFc8 _ OP_1 _ P(reg),   7,  NULL,  "fstp qword ptr %0\n")
rr(P(stmt),  OP_ASGNI4 _ OP_2 _
                 P(addr) _
                 OP_CVFIc4 _ OP_1 _ P(reg),  29,  NULL,  "fistp dword ptr %0\n")

rr(P(stmt),  OP_ASGNF4 _ OP_2 _ P(addr) _ P(reg),  7,  NULL,  "fstp dword ptr %0\n")
rr(P(stmt),  OP_ASGNF8 _ OP_2 _ P(addr) _ P(reg),  7,  NULL,  "fstp qword ptr %0\n")
rr(P(stmt),  OP_ASGNFc _ OP_2 _ P(addr) _ P(reg),  7,  NULL,  "fstp tbyte ptr %0\n")
rr(P(stmt),  OP_ASGNI1 _ OP_2 _ P(addr) _ P(rc),   1,  NULL,  "mov byte ptr %0,%1\n")
rr(P(stmt),  OP_ASGNI2 _ OP_2 _ P(addr) _ P(rc),   1,  NULL,  "mov word ptr %0,%1\n")
rr(P(stmt),  OP_ASGNI4 _ OP_2 _ P(addr) _ P(rc),   1,  NULL,  "mov dword ptr %0,%1\n")
rr(P(stmt),  OP_ASGNP4 _ OP_2 _ P(addr) _ P(rc),   1,  NULL,  "mov dword ptr %0,%1\n")
rr(P(stmt),  OP_ASGNB _ OP_2 _
                 P(reg) _
                 OP_INDIRB _ OP_1 _ P(reg),        0,  NULL,  "mov ecx,%a\n"
                                                              "rep movsb\n")

/* INDIR */
rr(P(memf),  OP_INDIRF4 _ OP_1 _ P(addr),  0,  NULL,  "dword ptr %0")
rr(P(memf),  OP_INDIRF8 _ OP_1 _ P(addr),  0,  NULL,  "qword ptr %0")
rr(P(memf),  OP_INDIRFc _ OP_1 _ P(addr),  0,  NULL,  "tbyte ptr %0")
rr(P(mem),   OP_INDIRI1 _ OP_1 _ P(addr),  0,  NULL,  "byte ptr %0")
rr(P(mem),   OP_INDIRI2 _ OP_1 _ P(addr),  0,  NULL,  "word ptr %0")
rr(P(mem),   OP_INDIRI4 _ OP_1 _ P(addr),  0,  NULL,  "dword ptr %0")
rr(P(mem),   OP_INDIRP4 _ OP_1 _ P(addr),  0,  NULL,  "dword ptr %0")

/* CVFF */
rr(P(reg),   OP_CVFF4c _ OP_1 _ P(reg),  0,  NULL,  "# CVFF4c\n")
rr(P(reg),   OP_CVFF8c _ OP_1 _ P(reg),  0,  NULL,  "# CVFF8c\n")

rr(P(memf),  OP_CVFF4c _ OP_1 _
                 OP_INDIRF4 _ OP_1 _ P(addr),  0,  NULL,  "dword ptr %0")
rr(P(memf),  OP_CVFF8c _ OP_1 _
                 OP_INDIRF8 _ OP_1 _ P(addr),  0,  NULL,  "qword ptr %0")

rr(P(reg),  OP_CVFFc4 _ OP_1 _ P(reg),  12,  NULL,  "sub esp,4\n"
                                                    "fstp dword ptr 0[esp]\n"
                                                    "fld dword ptr 0[esp]\n"
                                                    "add esp,4\n")
rr(P(reg),  OP_CVFFc8 _ OP_1 _ P(reg),  12,  NULL,  "sub esp,8\n"
                                                    "fstp qword ptr 0[esp]\n"
                                                    "fld qword ptr 0[esp]\n"
                                                    "add esp,8\n")

/* CVFI */
rr(P(reg),  OP_CVFIc4 _ OP_1 _ P(reg),  31,  NULL,  "sub esp,4\n"
                                                    "fistp dword ptr 0[esp]\n"
                                                    "pop %R\n")

/* CVIF */
rr(P(reg),  OP_CVIF4c _ OP_1 _
                OP_INDIRI4 _ OP_1 _ P(addr),  10,  NULL,  "fild dword ptr %0\n")
rr(P(reg),  OP_CVIF4c _ OP_1 _ P(reg),        12,  NULL,  "push %0\n"
                                                          "fild dword ptr 0[esp]\n"
                                                          "add esp,4\n")

/* CVII */
rr(P(reg),  OP_CVII14 _ OP_1 _
                OP_INDIRI1 _ OP_1 _ P(addr),  3,  NULL,  "movsx %R,byte ptr %0\n")
rr(P(reg),  OP_CVII24 _ OP_1 _
                OP_INDIRI2 _ OP_1 _ P(addr),  3,  NULL,  "movsx %R,word ptr %0\n")
rr(P(reg),  OP_CVII14 _ OP_1 _ P(reg),        3,  NULL,  "# extend\n")
rr(P(reg),  OP_CVII24 _ OP_1 _ P(reg),        3,  NULL,  "# extend\n")

rr(P(reg),  OP_CVII41 _ OP_1 _ P(reg),  1,  NULL,  "# truncate\n")
rr(P(reg),  OP_CVII42 _ OP_1 _ P(reg),  1,  NULL,  "# truncate\n")

/* CVIU */
rr(P(reg),  OP_CVIU14 _ OP_1 _
                OP_INDIRI1 _ OP_1 _ P(addr),  3,  NULL,          "movzx %R,byte ptr %0\n")
rr(P(reg),  OP_CVIU24 _ OP_1 _
                OP_INDIRI2 _ OP_1 _ P(addr),  3,  NULL,          "movzx %R,word ptr %0\n")
rr(P(reg),  OP_CVIU14 _ OP_1 _ P(reg),        3,  NULL,          "# extend\n")
rr(P(reg),  OP_CVIU24 _ OP_1 _ P(reg),        3,  NULL,          "# extend\n")
rr(P(reg),  OP_CVIU44 _ OP_1 _ P(reg),        0,  gen_notarget,  "%0")
rr(P(reg),  OP_CVIU44 _ OP_1 _ P(reg),        0,  gen_move,      "mov %R,%0\n")

/* CVUI */
rr(P(reg),  OP_CVUI41 _ OP_1 _ P(reg),  1,  NULL,          "# truncate\n")
rr(P(reg),  OP_CVUI42 _ OP_1 _ P(reg),  1,  NULL,          "# truncate\n")
rr(P(reg),  OP_CVUI44 _ OP_1 _ P(reg),  0,  gen_notarget,  "%0")
rr(P(reg),  OP_CVUI44 _ OP_1 _ P(reg),  0,  gen_move,      "mov %R,%0\n")

/* CVUU - not applicable */

/* CVUP */
rr(P(reg),  OP_CVUP44 _ OP_1 _ P(reg),  0,  gen_notarget,  "%0")
rr(P(reg),  OP_CVUP44 _ OP_1 _ P(reg),  0,  gen_move,      "mov %R,%0\n")

/* CVPU */
rr(P(reg),  OP_CVPU44 _ OP_1 _ P(reg),  0,  gen_notarget,  "%0")
rr(P(reg),  OP_CVPU44 _ OP_1 _ P(reg),  0,  gen_move,      "mov %R,%0\n")

/* NEG */
rr(P(reg),  OP_NEGF4 _ OP_1 _ P(reg),  0,  NULL,  "fchs\n")
rr(P(reg),  OP_NEGF8 _ OP_1 _ P(reg),  0,  NULL,  "fchs\n")
rr(P(reg),  OP_NEGFc _ OP_1 _ P(reg),  0,  NULL,  "fchs\n")
rr(P(reg),  OP_NEGI4 _ OP_1 _ P(reg),  2,  NULL,  "?mov %R,%0\n"
                                                  "neg %R\n")

/* CALL */
rr(P(stmt),  OP_CALLF4 _ OP_1 _ P(addrj),  0,  NULL,  "call %0\n"
                                                      "add esp,%a\n"
                                                      "fstp\n")
rr(P(stmt),  OP_CALLF8 _ OP_1 _ P(addrj),  0,  NULL,  "call %0\n"
                                                      "add esp,%a\n"
                                                      "fstp\n")
rr(P(stmt),  OP_CALLFc _ OP_1 _ P(addrj),  0,  NULL,  "call %0\n"
                                                      "add esp,%a\n"
                                                      "fstp\n")
rr(P(reg),   OP_CALLF4 _ OP_1 _ P(addrj),  0,  NULL,  "call %0\n"
                                                      "add esp,%a\n")
rr(P(reg),   OP_CALLF8 _ OP_1 _ P(addrj),  0,  NULL,  "call %0\n"
                                                      "add esp,%a\n")
rr(P(reg),   OP_CALLFc _ OP_1 _ P(addrj),  0,  NULL,  "call %0\n"
                                                      "add esp,%a\n")
rr(P(reg),   OP_CALLI4 _ OP_1 _ P(addrj),  0,  NULL,  "call %0\n"
                                                      "add esp,%a\n")
rr(P(stmt),  OP_CALLV _ OP_1 _ P(addrj),   0,  NULL,  "call %0\n"
                                                      "add esp,%a\n")

/* LOAD */
rr(P(reg),  OP_LOADI1 _ OP_1 _ P(reg),  0,  gen_move,  "mov %R,%0\n")
rr(P(reg),  OP_LOADI2 _ OP_1 _ P(reg),  0,  gen_move,  "mov %R,%0\n")
rr(P(reg),  OP_LOADI4 _ OP_1 _ P(reg),  0,  gen_move,  "mov %R,%0\n")
rr(P(reg),  OP_LOADU4 _ OP_1 _ P(reg),  0,  gen_move,  "mov %R,%0\n")
rr(P(reg),  OP_LOADP4 _ OP_1 _ P(reg),  0,  gen_move,  "mov %R,%0\n")

/* RET */
rr(P(stmt),  OP_RETF4 _ OP_1 _ P(reg),  0,  NULL,  "# ret\n")
rr(P(stmt),  OP_RETF8 _ OP_1 _ P(reg),  0,  NULL,  "# ret\n")
rr(P(stmt),  OP_RETFc _ OP_1 _ P(reg),  0,  NULL,  "# ret\n")
rr(P(stmt),  OP_RETI4 _ OP_1 _ P(reg),  0,  NULL,  "# ret\n")

/* ADDR */
rr(P(acon),   OP_ADDRGP4,  0,  NULL,  "%a")
rr(P(base),   OP_ADDRGP4,  0,  NULL,  "%a")
rr(P(addrj),  OP_ADDRGP4,  0,  NULL,  "%a")
rr(P(base),   OP_ADDRFP4,  0,  NULL,  "%a[ebp]")
rr(P(base),   OP_ADDRLP4,  0,  NULL,  "%a[ebp]")

/* ADD */
rr(P(base),  OP_ADDI4 _ OP_2 _ P(reg) _ P(acon),    0,  NULL,  "%1[%0]")
rr(P(base),  OP_ADDU4 _ OP_2 _ P(reg) _ P(acon),    0,  NULL,  "%1[%0]")
rr(P(base),  OP_ADDP4 _ OP_2 _ P(reg) _ P(acon),    0,  NULL,  "%1[%0]")

rr(P(addr),  OP_ADDI4 _ OP_2 _ P(index) _ P(base),  0,  NULL,  "%1[%0]")
rr(P(addr),  OP_ADDU4 _ OP_2 _ P(index) _ P(base),  0,  NULL,  "%1[%0]")
rr(P(addr),  OP_ADDP4 _ OP_2 _ P(index) _ P(base),  0,  NULL,  "%1[%0]")

rr(P(reg),  OP_ADDF4 _ OP_2 _ P(reg) _ P(flt),   0,  NULL,  "fadd%1\n")
rr(P(reg),  OP_ADDF8 _ OP_2 _ P(reg) _ P(flt),   0,  NULL,  "fadd%1\n")
rr(P(reg),  OP_ADDFc _ OP_2 _ P(reg) _ P(flt),   0,  NULL,  "fadd%1\n")
rr(P(reg),  OP_ADDI4 _ OP_2 _ P(reg) _ P(mrc1),  1,  NULL,  "?mov %R,%0\n"
                                                            "add %R,%1\n")
rr(P(reg),  OP_ADDU4 _ OP_2 _ P(reg) _ P(mrc1),  1,  NULL,  "?mov %R,%0\n"
                                                            "add %R,%1\n")
rr(P(reg),  OP_ADDP4 _ OP_2 _ P(reg) _ P(mrc1),  1,  NULL,  "?mov %R,%0\n"
                                                            "add %R,%1\n")

/* SUB */
rr(P(reg),  OP_SUBF4 _ OP_2 _ P(reg) _ P(flt),   0,  NULL,  "fsub%1\n")
rr(P(reg),  OP_SUBF8 _ OP_2 _ P(reg) _ P(flt),   0,  NULL,  "fsub%1\n")
rr(P(reg),  OP_SUBFc _ OP_2 _ P(reg) _ P(flt),   0,  NULL,  "fsub%1\n")
rr(P(reg),  OP_SUBI4 _ OP_2 _ P(reg) _ P(mrc1),  1,  NULL,  "?mov %R,%0\n"
                                                            "sub %R,%1\n")
rr(P(reg),  OP_SUBU4 _ OP_2 _ P(reg) _ P(mrc1),  1,  NULL,  "?mov %R,%0\n"
                                                            "sub %R,%1\n")
rr(P(reg),  OP_SUBP4 _ OP_2 _ P(reg) _ P(mrc1),  1,  NULL,  "?mov %R,%0\n"
                                                            "sub %R,%1\n")

/* LSH */
rr(P(index),  OP_LSHI4 _ OP_2 _ P(reg) _ P(con1),  0,  NULL,  "%0*2")
rr(P(index),  OP_LSHI4 _ OP_2 _ P(reg) _ P(con2),  0,  NULL,  "%0*4")
rr(P(index),  OP_LSHI4 _ OP_2 _ P(reg) _ P(con3),  0,  NULL,  "%0*8")
rr(P(index),  OP_LSHU4 _ OP_2 _ P(reg) _ P(con1),  0,  NULL,  "%0*2")
rr(P(index),  OP_LSHU4 _ OP_2 _ P(reg) _ P(con2),  0,  NULL,  "%0*4")
rr(P(index),  OP_LSHU4 _ OP_2 _ P(reg) _ P(con3),  0,  NULL,  "%0*8")
rr(P(reg),    OP_LSHI4 _ OP_2 _ P(reg) _ P(rc5),   2,  NULL,  "?mov %R,%0\n"
                                                              "sal %R,%1\n")
rr(P(reg),    OP_LSHU4 _ OP_2 _ P(reg) _ P(rc5),   2,  NULL,  "?mov %R,%0\n"
                                                              "shl %R,%1\n")

/* MOD */
rr(P(reg),  OP_MODI4 _ OP_2 _ P(reg) _ P(reg),  0,  NULL,  "cdq\n"
                                                           "idiv %1\n")
rr(P(reg),  OP_MODU4 _ OP_2 _ P(reg) _ P(reg),  0,  NULL,  "xor edx,edx\n"
                                                           "div %1\n")

/* RSH */
rr(P(reg),  OP_RSHI4 _ OP_2 _ P(reg) _ P(rc5),  2,  NULL,  "?mov %R,%0\n"
                                                           "sar %R,%1\n")
rr(P(reg),  OP_RSHU4 _ OP_2 _ P(reg) _ P(rc5),  2,  NULL,  "?mov %R,%0\n"
                                                           "shr %R,%1\n")

/* BAND */
rr(P(reg),  OP_BANDU4 _ OP_2 _ P(reg) _ P(mrc1),  1,  NULL,  "?mov %R,%0\n"
                                                             "and %R,%1\n")

/* BCOM */
rr(P(reg),  OP_BCOMU4 _ OP_1 _ P(reg),  2,  NULL,  "?mov %R,%0\n"
                                                   "not %R\n")

/* BOR */
rr(P(reg),  OP_BORU4 _ OP_2 _ P(reg) _ P(mrc1),  1,  NULL,  "?mov %R,%0\n"
                                                            "or %R,%1\n")

/* BXOR */
rr(P(reg),  OP_BXORU4 _ OP_2 _ P(reg) _ P(mrc1),  1,  NULL,  "?mov %R,%0\n"
                                                             "xor %R,%1\n")

/* DIV */
rr(P(reg),  OP_DIVF4 _ OP_2 _ P(reg) _ P(flt),  0,  NULL,  "fdiv%1\n")
rr(P(reg),  OP_DIVF8 _ OP_2 _ P(reg) _ P(flt),  0,  NULL,  "fdiv%1\n")
rr(P(reg),  OP_DIVFc _ OP_2 _ P(reg) _ P(flt),  0,  NULL,  "fdiv%1\n")
rr(P(reg),  OP_DIVI4 _ OP_2 _ P(reg) _ P(reg),  0,  NULL,  "cdq\n"
                                                           "idiv %1\n")
rr(P(reg),  OP_DIVU4 _ OP_2 _ P(reg) _ P(reg),  0,  NULL,  "xor edx,edx\n"
                                                           "div %1\n")

/* MUL */
rr(P(reg),  OP_MULF4 _ OP_2 _ P(reg) _ P(flt),    0,  NULL,  "fmul%1\n")
rr(P(reg),  OP_MULF8 _ OP_2 _ P(reg) _ P(flt),    0,  NULL,  "fmul%1\n")
rr(P(reg),  OP_MULFc _ OP_2 _ P(reg) _ P(flt),    0,  NULL,  "fmul%1\n")
rr(P(reg),  OP_MULI4 _ OP_2 _ P(reg) _ P(mrc3),  14,  NULL,  "?mov %R,%0\n"
                                                             "imul %R,%1\n")
rr(P(reg),  OP_MULI4 _ OP_2 _ P(con) _ P(mr),    13,  NULL,  "imul %R,%1,%0\n")
rr(P(reg),  OP_MULU4 _ OP_2 _ P(reg) _ P(mrc3),  14,  NULL,  "?mov %R,%0\n"
                                                             "imul %R,%1\n")
rr(P(reg),  OP_MULU4 _ OP_2 _ P(con) _ P(mr),    13,  NULL,  "imul %R,%1,%0\n")

/* EQ */
rr(P(stmt),  OP_EQF4 _ OP_2 _ P(cmpf) _ P(reg),  0,  NULL,  "fcomp%0\n"
                                                            "fstsw ax\n"
                                                            "sahf\n"
                                                            "je %a\n")
rr(P(stmt),  OP_EQF8 _ OP_2 _ P(cmpf) _ P(reg),  0,  NULL,  "fcomp%0\n"
                                                            "fstsw ax\n"
                                                            "sahf\n"
                                                            "je %a\n")
rr(P(stmt),  OP_EQFc _ OP_2 _ P(cmpf) _ P(reg),  0,  NULL,  "fcomp%0\n"
                                                            "fstsw ax\n"
                                                            "sahf\n"
                                                            "je %a\n")
rr(P(stmt),  OP_EQI4 _ OP_2 _ P(mem) _ P(rc),    5,  NULL,  "cmp %0,%1\n"
                                                            "je %a\n")
rr(P(stmt),  OP_EQI4 _ OP_2 _ P(reg) _ P(mrc1),  4,  NULL,  "cmp %0,%1\n"
                                                            "je %a\n")

/* GE */
rr(P(stmt),  OP_GEF4 _ OP_2 _ P(cmpf) _ P(reg),  0,  NULL,  "fcomp%0\n"
                                                            "fstsw ax\n"
                                                            "sahf\n"
                                                            "jbe %a\n")
rr(P(stmt),  OP_GEF8 _ OP_2 _ P(cmpf) _ P(reg),  0,  NULL,  "fcomp%0\n"
                                                            "fstsw ax\n"
                                                            "sahf\n"
                                                            "jbe %a\n")
rr(P(stmt),  OP_GEFc _ OP_2 _ P(cmpf) _ P(reg),  0,  NULL,  "fcomp%0\n"
                                                            "fstsw ax\n"
                                                            "sahf\n"
                                                            "jbe %a\n")
rr(P(stmt),  OP_GEI4 _ OP_2 _ P(mem) _ P(rc),    5,  NULL,  "cmp %0,%1\n"
                                                            "jge %a\n")
rr(P(stmt),  OP_GEI4 _ OP_2 _ P(reg) _ P(mrc1),  4,  NULL,  "cmp %0,%1\n"
                                                            "jge %a\n")
rr(P(stmt),  OP_GEU4 _ OP_2 _ P(mem) _ P(rc),    5,  NULL,  "cmp %0,%1\n"
                                                            "jae %a\n")
rr(P(stmt),  OP_GEU4 _ OP_2 _ P(reg) _ P(mrc1),  4,  NULL,  "cmp %0,%1\n"
                                                            "jae %a\n")

/* GT */
rr(P(stmt),  OP_GTF4 _ OP_2 _ P(cmpf) _ P(reg),  0,  NULL,  "fcomp%0\n"
                                                            "fstsw ax\n"
                                                            "sahf\n"
                                                            "jb %a\n")
rr(P(stmt),  OP_GTF8 _ OP_2 _ P(cmpf) _ P(reg),  0,  NULL,  "fcomp%0\n"
                                                            "fstsw ax\n"
                                                            "sahf\n"
                                                            "jb %a\n")
rr(P(stmt),  OP_GTFc _ OP_2 _ P(cmpf) _ P(reg),  0,  NULL,  "fcomp%0\n"
                                                            "fstsw ax\n"
                                                            "sahf\n"
                                                            "jb %a\n")
rr(P(stmt),  OP_GTI4 _ OP_2 _ P(mem) _ P(rc),    5,  NULL,  "cmp %0,%1\n"
                                                            "jg %a\n")
rr(P(stmt),  OP_GTI4 _ OP_2 _ P(reg) _ P(mrc1),  4,  NULL,  "cmp %0,%1\n"
                                                            "jg %a\n")
rr(P(stmt),  OP_GTU4 _ OP_2 _ P(mem) _ P(rc),    5,  NULL,  "cmp %0,%1\n"
                                                            "ja %a\n")
rr(P(stmt),  OP_GTU4 _ OP_2 _ P(reg) _ P(mrc1),  4,  NULL,  "cmp %0,%1\n"
                                                            "ja %a\n")

/* LE */
rr(P(stmt),  OP_LEF4 _ OP_2 _ P(cmpf) _ P(reg),  0,  NULL,  "fcomp%0\n"
                                                            "fstsw ax\n"
                                                            "sahf\n"
                                                            "jae %a\n")
rr(P(stmt),  OP_LEF8 _ OP_2 _ P(cmpf) _ P(reg),  0,  NULL,  "fcomp%0\n"
                                                            "fstsw ax\n"
                                                            "sahf\n"
                                                            "jae %a\n")
rr(P(stmt),  OP_LEFc _ OP_2 _ P(cmpf) _ P(reg),  0,  NULL,  "fcomp%0\n"
                                                            "fstsw ax\n"
                                                            "sahf\n"
                                                            "jae %a\n")
rr(P(stmt),  OP_LEI4 _ OP_2 _ P(mem) _ P(rc),    5,  NULL,  "cmp %0,%1\n"
                                                            "jle %a\n")
rr(P(stmt),  OP_LEI4 _ OP_2 _ P(reg) _ P(mrc1),  4,  NULL,  "cmp %0,%1\n"
                                                            "jle %a\n")
rr(P(stmt),  OP_LEU4 _ OP_2 _ P(mem) _ P(rc),    5,  NULL,  "cmp %0,%1\n"
                                                            "jbe %a\n")
rr(P(stmt),  OP_LEU4 _ OP_2 _ P(reg) _ P(mrc1),  4,  NULL,  "cmp %0,%1\n"
                                                            "jbe %a\n")

/* LT */
rr(P(stmt),  OP_LTF4 _ OP_2 _ P(cmpf) _ P(reg),  0,  NULL,  "fcomp%0\n"
                                                            "fstsw ax\n"
                                                            "sahf\n"
                                                            "ja %a\n")
rr(P(stmt),  OP_LTF8 _ OP_2 _ P(cmpf) _ P(reg),  0,  NULL,  "fcomp%0\n"
                                                            "fstsw ax\n"
                                                            "sahf\n"
                                                            "ja %a\n")
rr(P(stmt),  OP_LTFc _ OP_2 _ P(cmpf) _ P(reg),  0,  NULL,  "fcomp%0\n"
                                                            "fstsw ax\n"
                                                            "sahf\n"
                                                            "ja %a\n")
rr(P(stmt),  OP_LTI4 _ OP_2 _ P(mem) _ P(rc),    5,  NULL,  "cmp %0,%1\n"
                                                            "jl %a\n")
rr(P(stmt),  OP_LTI4 _ OP_2 _ P(reg) _ P(mrc1),  4,  NULL,  "cmp %0,%1\n"
                                                            "jl %a\n")
rr(P(stmt),  OP_LTU4 _ OP_2 _ P(mem) _ P(rc),    5,  NULL,  "cmp %0,%1\n"
                                                            "jb %a\n")
rr(P(stmt),  OP_LTU4 _ OP_2 _ P(reg) _ P(mrc1),  4,  NULL,  "cmp %0,%1\n"
                                                            "jb %a\n")

/* NE */
rr(P(stmt),  OP_NEF4 _ OP_2 _ P(cmpf) _ P(reg),  0,  NULL,  "fcomp%0\n"
                                                            "fstsw ax\n"
                                                            "sahf\n"
                                                            "jne %a\n")
rr(P(stmt),  OP_NEF8 _ OP_2 _ P(cmpf) _ P(reg),  0,  NULL,  "fcomp%0\n"
                                                            "fstsw ax\n"
                                                            "sahf\n"
                                                            "jne %a\n")
rr(P(stmt),  OP_NEFc _ OP_2 _ P(cmpf) _ P(reg),  0,  NULL,  "fcomp%0\n"
                                                            "fstsw ax\n"
                                                            "sahf\n"
                                                            "jne %a\n")
rr(P(stmt),  OP_NEI4 _ OP_2 _ P(mem) _ P(rc),    5,  NULL,  "cmp %0,%1\n"
                                                            "jne %a\n")
rr(P(stmt),  OP_NEI4 _ OP_2 _ P(reg) _ P(mrc1),  4,  NULL,  "cmp %0,%1\n"
                                                            "jne %a\n")

/* JMP */
rr(P(stmt),  OP_JMPV _ OP_1 _ P(addrj),  3,  NULL,  "jmp %0\n")

/* LABEL */
rr(P(stmt),  OP_LABELV,  0,  NULL,  "%a:\n")

/* chain rules */
rr(P(acon),   P(con),  0,  NULL,  "%0")
rr(P(base),   P(reg),  0,  NULL,  "[%0]")
rr(P(index),  P(reg),  0,  NULL,  "%0")

rr(P(addr),  P(base),   0,  NULL,  "%0")
rr(P(addr),  P(index),  0,  NULL,  "[%0]")

rr(P(rc),  P(reg),  0,  NULL,  "%0")
rr(P(rc),  P(con),  0,  NULL,  "%0")

rr(P(mr),  P(reg),  0,  NULL,  "%0")
rr(P(mr),  P(mem),  0,  NULL,  "%0")

rr(P(mrc0),  P(mem),  0,  NULL,  "%0")
rr(P(mrc0),  P(rc),   0,  NULL,  "%0")
rr(P(mrc1),  P(mem),  1,  NULL,  "%0")
rr(P(mrc1),  P(rc),   0,  NULL,  "%0")
rr(P(mrc3),  P(mem),  3,  NULL,  "%0")
rr(P(mrc3),  P(rc),   0,  NULL,  "%0")

rr(P(reg),  P(addr),  1,  NULL,  "lea %R,%0\n")
rr(P(reg),  P(mrc0),  1,  NULL,  "mov %R,%0\n")
rr(P(rc5),  P(reg),   0,  NULL,  "cl")

rr(P(reg),  P(memf),  3,  NULL,  "fld %0\n")
rr(P(flt),  P(memf),  0,  NULL,  " %0")
rr(P(flt),  P(reg),   0,  NULL,  "p st(1),st")

rr(P(addrj),  P(reg),  2,  NULL,  "%0")
rr(P(addrj),  P(mem),  2,  NULL,  "%0")

rr(P(cmpf),  P(memf),  0,  NULL,  " %0")
rr(P(cmpf),  P(reg),   0,  NULL,  "p")

#undef tt
#undef rr

/* end of bx86t.r */
