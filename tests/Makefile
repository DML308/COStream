PARSE = ../c2c
PARSEFLAGS = -Wall# -notrans -DNO_STDARG -DNO_STRTOUL -DNO_PROTOTYPES
CC = gcc
CFLAGS = -Wall# -DNO_STDARG -DNO_STRTOUL -DNO_PROTOTYPES

KNOWN_UNFIXABLE_BUGS = enum.err ptrarith.err extern.err

ERROR_MESSAGE_TESTS = \
   const.err cfold.err signed.err undecl.err coerce.err \
   array-field.err switch.err tq.err init.err anonstruct.err escapes.err \
   A.err B.err char.err expr.err old.err proc.err \
   scope.err test.err tricky.err \
   empty.err enumargs.err unused.err preced.err voidptr.err bt.err sdcls.err \
   callargs.err arr.err include.err f.err 

ANALYSIS_TESTS = \
   analysis.an

TRANSLATION_TESTS =  \
   unsign.ast enumconst.ast tdef.ast weirdtypes.ast label.ast sizeofstr.ast \
   int.ast nasty.ast S.ast  \
   cpp.ast fact.ast string-concat.ast nodim.ast sizeof.ast switch2.ast \
   cond.ast ec.ast tdefreturn.ast struct.ast bitfield.ast  \
   undim.ast gnu.ast ifelse.ast attrib.ast

C2C_SOURCE =  \
   ../ast.end            ../complex-types.end  ../container.end    \
   ../constexpr.end      ../conversions.end    ../initializer.end  \
   ../list.end           ../main.end           ../operators.end    \
   ../output.end         ../print-ast.end      ../procedure.end    \
   ../sem-check.end      ../strings.end        ../sue.end          \
   ../symbol.end         ../transform.end      ../type.end         \
   ../type2.end          ../verify-parse.end   ../warning.end      \
   ../dataflow.end       ../analyze.end


.SUFFIXES: .msg .ast .end .err .an

CLEAN = rm -f *.p.c *.ast *~ *.BAK *.bak *.orig *.o *.out *.err .\#*


all: error-messages translation analysis self-parse


error-messages: $(ERROR_MESSAGE_TESTS)

analysis: $(ANALYSIS_TESTS)

translation: $(TRANSLATION_TESTS)


self-parse: $(C2C_SOURCE)


known-bugs: $(KNOWN_UNFIXABLE_BUGS)

clean:
	$(CLEAN)




.c.ast:
	@echo "--------------------------------- AST test: $<"
	$(PARSE) $(PARSEFLAGS) -ast -o $(*F).p.c $< | sed -e "s/0x[0-9a-fA-F]*//g" > $*.ast
	$(PARSE) $(PARSEFLAGS) -ast -o $(*F).p.p.c $(*F).p.c | sed -e "s/0x[0-9a-fA-F]*//g" > $(*F).p.ast
	diff $(*F).ast $(*F).p.ast
	diff $(*F).p.c $(*F).p.p.c
	@echo ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> $< tested successfully."
	@echo

.c.err:
	@echo "--------------------------------- Error-message test: $<"
	-$(PARSE) $(PARSEFLAGS) $< > $(*F).err 2>&1
	diff $(*F).err $(*F).msg
	@echo ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> $< tested successfully."
	@echo

.c.an:
	@echo "--------------------------------- Analysis test: $<"
	$(PARSE) $(PARSEFLAGS) -live -o $(*F).p.c $< > $*.err 2>&1
	diff $(*F).err $(*F).msg
	diff $(*F).p.c $(*F).a.c
	@echo ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> $< tested successfully."
	@echo


.c.end:
	@echo "--------------------------------- End-to-end test: $<"
	cp $< $(*F).p.c
	$(CC) $(CFLAGS) -I.. -c -o $(*F).o $(*F).p.c
	$(PARSE) $(PARSEFLAGS) -I.. -o $(*F).p.c $(*F).p.c
	$(CC) $(CFLAGS) -I.. -c -o $(*F).p.o $(*F).p.c
	diff $(*F).o $(*F).p.o
	@echo ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> $< tested successfully."
	@echo


.c.msg:
	-$(PARSE) $(PARSEFLAGS) $< >$@ 2>&1 
