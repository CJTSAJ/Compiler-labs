#include <stdio.h>
#include <stdlib.h>
#include "util.h"
#include "symbol.h"
#include "absyn.h"
#include "temp.h"
#include "errormsg.h"
#include "tree.h"
#include "assem.h"
#include "frame.h"
#include "codegen.h"
#include "table.h"

//Lab 6: put your code here
static void emit(AS_instr inst);
static void munchStm(T_stm stm);
static Temp_temp munchExp(T_exp exp);
static Temp_TempList munchArgs(int cnt, T_expList args);

//helper func
static bool isPlainExp(T_exp e){
    return e->kind == T_CONST || e->kind == T_TEMP ;
}
static void MemOp(Temp_temp src, Temp_temp dst, T_exp e){
    switch(e->kind){
        case T_CONST:{
            char mstr[100];
            sprintf(mstr,"movq %d('s0), 'd0", e->u.CONST);
            emit(AS_Move(mstr, Temp_TempList(dst,NULL), Temp_TempList(src,NULL)));
        }
        case T_TEMP:{
            string mstr = "movq ('s0,'s1) 'd0";
            emit(AS_Move(mstr, Temp_TempList(dst,NULL), Temp_TempList(src,Temp_TempList(e->u.TEMP, NULL))));
        }
    }
}
static void MovOp(Temp_temp src, Temp_temp dst, T_exp e){
    switch(e->kind){
        case T_CONST:{
            char mstr[100];
            sprintf(mstr,"movq 's0, %d('d0)", e->u.CONST);
            emit(AS_Move(mstr, Temp_TempList(dst,NULL), Temp_TempList(src,NULL)));
        }
        case T_TEMP:{
            string mstr = "movq 's0, ('d0,'d1)";
            emit(AS_Move(mstr, Temp_TempList(dst,Temp_TempList(e->u.TEMP, NULL)), Temp_TempList(src,NULL)));
        }
    }
}

static void munchStm(T_stm stm){
    switch(stm->kind){
        case T_SEQ:{
            EM_impossible("T_SEQ stm should not exist");
            return;            
        }
        case T_LABEL:{
            Temp_label LABEL = stm->u.LABEL;
            emit(AS_Label(Temp_labelstring(LABEL), LABEL));
            return;
        }
        case T_JUMP:{
            T_exp expp = stm->u.JUMP.exp; 
            Temp_labelList jumps = stm->u.JUMP.jumps;
            if(expp->kind != T_NAME){
                EM_impossible("T_JUMP should have a T_NAME exp");
                return;
            }
            emit(AS_Oper("jmp 'j0", NULL, NULL, AS_Targets(jumps)));
            return;
        }
        case T_CJUMP:{
            T_relOp op = stm->u.CJUMP.op;            
			Temp_label t = stm->u.CJUMP.true;
            Temp_temp left = munchExp(stm->u.CJUMP.left);
            Temp_temp right = munchExp(stm->u.CJUMP.right);
            emit(AS_Oper("cmpq 's0, 's1", NULL, 
                    Temp_TempList(right, Temp_TempList(left, NULL)),NULL));
            string jstr;
            switch(op){
                case T_eq:jstr="je 'j0";break;
                case T_ne:jstr="jne 'j0";break;
                case T_lt:jstr="jl 'j0";break;
                case T_gt:jstr="jg 'j0";break;
                case T_le:jstr="jle 'j0";break;
                case T_ge:jstr="jge 'j0";break;
		        case T_ult:jstr="jb 'j0";break;
                case T_ule:jstr="jbe 'j0";break;
                case T_ugt:jstr="ja 'j0";break;
                case T_uge:jstr="jae 'j0";break;
            }
            emit(AS_Oper(jstr, NULL, NULL, AS_Targets(Temp_LabelList(t, NULL))));
            return;
        }
        case T_EXP:{
            Temp_temp s = munchExp(stm->u.EXP);
            Temp_temp d = Temp_newtemp();
            emit(AS_Move("movq 's0, 'd0", Temp_TempList(d,NULL), Temp_TempList(s,NULL)));
            return;
        }
        case T_MOVE:{
            Temp_temp s = munchExp(stm->u.MOVE.src);
            T_exp dst = stm->u.MOVE.dst;
            if(dst->kind == T_Temp ){
                emit(AS_Move("movq 's0, 'd0", Temp_TempList(dst->u.TEMP,NULL), Temp_TempList(s,NULL)));
                return;
            }
            if(dst->kind == T_MEM){
                T_exp MEM = dst->u.MEM;
                if(MEM->kind == T_TEMP){
                    Temp_temp TEMP = MEM->u.TEMP;
                    emit(AS_Move("movq 's0, ('d0)", Temp_TempList(TEMP,NULL),Temp_TempList(s,NULL)));
                    return;
                }     
                if(MEM->kind == T_CONST){
                    Temp_temp d = munchExp(MEM);
                    emit(AS_Move("movq 's0, ('d0)", Temp_TempList(d,NULL),Temp_TempList(s,NULL)));
                    return;
                }       
                if(MEM->kind == T_BINOP){
                    T_binOp op = MEM->u.BINOP.op;
                    T_exp left = MEM->u.BINOP.left;
                    T_exp right = MEM->u.BINOP.right;
                    if(!isPlainExp(left) && !isPlainExp(right)){
                        Temp_temp d = munchExp(MEM);
                        emit(AS_Move("movq 's0, ('d0)", Temp_TempList(d,NULL),Temp_TempList(s,NULL)));
                        return dst;
                    }
                    if(!isPlainExp(left)){
                        Temp_temp d = munchExp(left);
                        MovOp(s,d,right);
                        return;
                    }
                    if(!isPlainExp(right)){
                        Temp_temp d = munchExp(right);
                        MemOp(s, d, left);
                        return;
                    }
                    else{
                        if(right->kind == T_Temp)
                            MovOp(s, right->u.TEMP, left);
                        else if(left->kind == T_Temp)
                            MovOp(s, left->u.TEMP, right);
                        else
                            EM_impossible("unexpected MEM(CONST op CONST)");assert(0);
                        return;
                    }
                }
            }
            EM_impossible("T_MOVE dst must be T_MEM or T_TEMP");assert(0);
            return;
        }
	    
    }
}
static Temp_temp munchExp(T_exp e){
    switch(e->kind){        
        case T_BINOP:{  /*op S, D  :  D = D op S */
            T_binOp op = e->u.BINOP.op;
            Temp_temp left = munchExp(e->u.BINOP.left);
            Temp_temp right = munchExp(e->u.BINOP.right);
            string opstr;
            switch(op){
               case T_plus:opstr = "addq 's1, 'd0";break;
               case T_minus:opstr = "subq 's1, 'd0";break;
               case T_mul:opstr = "imulq 's1, 'd0";break;
               case T_div: case T_and: case T_or:
               case T_lshift: case T_rshift: case T_arshift:
               case T_xor:EM_impossible("unsupported T_Binop operation");assert(0);
            }
            emit(AS_Oper(opstr, Temp_TempList(left, NULL),
                        Temp_TempList(left, Temp_TempList(right,NULL)), NULL));
            return right;
        }
        case T_MEM:{
            T_exp MEM = e->u.MEM;
            Temp_temp dst = Temp_newtemp();
            if(MEM->kind == T_TEMP){
                Temp_temp TEMP = MEM->u.TEMP;
                emit(AS_Move("movq ('s0), 'd0", Temp_TempList(dst,NULL),Temp_TempList(TEMP,NULL)));
                return dst;
            }     
            if(MEM->kind == T_CONST){
                Temp_temp src = munchExp(MEM);
                emit(AS_Move("movq ('s0), 'd0", Temp_TempList(dst,NULL),Temp_TempList(src,NULL)));
                return dst;
            }       
            if(MEM->kind == T_BINOP){
                T_binOp op = MEM->u.BINOP.op;
                T_exp left = MEM->u.BINOP.left;
                T_exp right = MEM->u.BINOP.right;
                if(!isPlainExp(left) && !isPlainExp(right)){
                    Temp_temp src = munchExp(MEM);
                    emit(AS_Move("movq ('s0), 'd0", Temp_TempList(dst,NULL),Temp_TempList(src,NULL)));
                    return dst;
                }
                if(!isPlainExp(left)){
                    Temp_temp src = munchExp(left);
                    MemOp(src, dst, right);
                    return dst;
                }
                if(!isPlainExp(right)){
                    Temp_temp src = munchExp(right);
                    MemOp(src, dst, left);
                    return dst;
                }
                else{
                    if(right->kind == T_Temp)
                        MemOp(right->u.TEMP, dst, left);
                    else if(left->kind == T_Temp)
                        MemOp(left->u.TEMP, dst, right);
                    else
                        EM_impossible("unexpected MEM(CONST op CONST)");assert(0);
                    return dst;
                }
            }
            else{
                EM_impossible("unexpected MEM( exp )");
                assert(0);
            }
        }
        case T_TEMP:{
            return e->u.TEMP;
        }
        case T_ESEQ:{
            EM_impossible("T_ESEQ exp should not exist");
            assert(0);
        }
        case T_NAME:{
            EM_impossible("T_NAME exp should exist in JUMP/SJUMP/CALL, or string representation wait to be complement");
            assert(0);
        }
		case T_CONST:{
            Temp_temp d = Temp_newtemp();
            char cstr[100];
            sprintf(cstr, "move $%d, 'd0", e->u.CONST);
            emit(AS_Oper(cstr, Temp_TempList(d,NULL), NULL, NULL));
            return d;
        }
        case T_CALL:{
            T_exp fun = e->u.CALL.fun; 
            T_expList args = e->u.CALL.args;
            munchArgs(0, args);
            if(fun->kind != T_NAME){
                EM_impossible("T_CALL func name must be a label");assert(0);
            }
            char call[100];
            sprintf(call, "call %s", Temp_labelstring(fun->u.NAME));
            emit(AS_Oper(call, NULL, NULL, NULL));
        }
    }
}

static Temp_tempList munchArgs(int cnt, T_expList args){
    if(!args)
        return NULL;
    
    T_exp arg = args->head;
    Temp_temp src = munchExp(arg);
    if(cnt < 6){
        Temp_temp dst = F_ARG(cnt);
        Temp_tempList tl = Temp_TempList(dst, munchArgs(cnt+1, args->tail));
        emit(AS_Oper("pushq 's0", NULL, Temp_TempList(dst, NULL), NULL));
        emit(AS_Move("movq 's0, 'd0", Temp_TempList(dst,NULL),emp_TempList(src,NULL)));
        return tl;
    }
    else{
        munchArgs(cnt+1, args->tail);
    }

}




static AS_instrList asList, Last;
static void emit(AS_instr inst) {
    if(!Last){
        Last = AS_InstrList(inst, NULL);
        asList = Last;
    }
    else{
        Last->tail = AS_InstrList(inst, NULL);
        Last = Last->tail;
    }
}

AS_instrList F_codegen(F_frame f, T_stmList stmList) {
    asList = NULL;
    Last = NULL;
    AS_instrList list; 

    /*miscellaneous initializations as necessary */

    for (T_stmList sl=stmList; sl; sl = sl->tail){
        T_stm stm = sl->head;
        munchStm(stm);
    }  
    list = asList;
    return list ;
}
