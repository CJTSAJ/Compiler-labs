#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"
#include "symbol.h"
#include "temp.h"
#include "table.h"
#include "tree.h"
#include "frame.h"

/*Lab5: Your implementation here.*/

const int F_wordsize = 8;

//F_frame_ in PPT
struct F_frame_ {
	Temp_label name;
	
	F_accessList formals;
	F_accessList locals;

	//the number of arguments
	int argSize;
	
	//the number of local variables
	int length;
	
	//register lists for the frame
	Temp_tempList calleesaves;
	Temp_tempList callersaves;
	Temp_tempList specialregs;
};

//varibales
struct F_access_ {
	enum {inFrame, inReg} kind;
	union {
		int offset; //inFrame
		Temp_temp reg; //inReg
	} u;
};

/* functions */
static F_access InFrame(int offset){
	F_access ac = checked_malloc(sizeof(*ac));

	ac->kind = inFrame;
	ac->u.offset = offset;
	return ac;
}   

static F_access InReg(Temp_temp reg){
	F_access ac = checked_malloc(sizeof(*ac));

	ac->kind = inReg;
	ac->u.reg = reg;
	return ac;
}

F_accessList F_AccessList(F_access head, F_accessList tail){
	F_accessList l = checked_malloc(sizeof(*l));

	l->head = head;
	l->tail = tail;
	return l;
}

// param position wait to be reset
F_accessList makeFormalsF(U_boolList formals, int* cntp){
	if(!formals){
		return NULL;
	}
	bool esc = formals->head;
	int cnt = *cntp;
	*cntp = cnt+1;

	F_access ac;
	if(esc){
		ac = InFrame(cnt * F_wordsize);
	}
	else{
		ac = InReg(Temp_newtemp());
	}
	
	if(formals->tail){
		return F_AccessList(ac, makeFormalsF(formals->tail, cntp));
	}
	else{
		return F_AccessList(ac, NULL);
	}
}

F_frame F_newFrame(Temp_label name, U_boolList formals){
	F_frame f = checked_malloc(sizeof(*f));
	int *argsize = checked_malloc(sizeof(int));
	*argsize = 0;
	
	f->name = name;
	f->formals = makeFormalsF(formals, argsize);
	f->locals = NULL;
	f->argSize = *argsize;
	f->length = 0;

	f->calleesaves = NULL;
	f->callersaves = NULL;
	f->specialregs = NULL;

	return f;
}

//locals position wait to be reset
F_access F_allocLocal(F_frame f, bool escape){
	int length = f->length;
	F_accessList locals = f->locals;

	F_access ac;
	if(escape){
		ac = InFrame(-(length+1) * F_wordsize);
	}
	else{
		ac = InReg(Temp_newtemp());
	}

	f->length = length+1;
	f->locals = F_AccessList(ac, locals);

	return ac;
}

Temp_label F_name(F_frame f){
	return f->name;
}
F_accessList F_formals(F_frame f){
	return f->formals;
}

/* IR translation */
Temp_temp F_FP(void){
	return Temp_newtemp();
}
Temp_temp F_SP(void){
	return Temp_newtemp();
}
Temp_temp F_RV(void){
	return Temp_newtemp();
}
Temp_temp F_PC(void){
	return Temp_newtemp();
}

T_exp F_exp(F_access acc, T_exp framePtr){
	if(acc->kind == inFrame){
		int off = acc->u.offset;
		return T_Mem(T_Binop(T_plus, T_Const(off), framePtr));
	}
	else{
		return T_Temp(acc->u.reg);
	}
}

T_exp F_externalCall(string s, T_expList args){
	return T_Call(T_Name(Temp_namedlabel(s)), args);
}

T_stm F_procEntryExit1(F_frame f, T_stm stm){//4, 5, 8
	return stm;
}
T_stm F_procEntryExit3(F_frame f, T_stm stm){//1, 3, 9, 11
	return stm;
}

/* fragment */
F_frag F_StringFrag(Temp_label label, string str) { 
	F_frag f = checked_malloc(sizeof(*f));
	f->kind = F_stringFrag;
	f->u.stringg.label = label;
	f->u.stringg.str = str;

	return f;                                      
}                                                     
                                                      
F_frag F_ProcFrag(T_stm body, F_frame frame) {        
	F_frag f = checked_malloc(sizeof(*f));
	f->kind = F_procFrag;
	f->u.proc.body = body;
	f->u.proc.frame = frame;

	return f;                                     
}                                                     
                                                      
F_fragList F_FragList(F_frag head, F_fragList tail) { 
	F_fragList l = checked_malloc(sizeof(*l));
	l->head = head;
	l->tail = tail;
	return l;                                      
}                                                     

