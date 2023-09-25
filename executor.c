#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "core.h"
#include "tree.h"
#include "executor.h"
#include "scanner.h"
#include "memory.h"



/*
*
* Execute functions
*
*/

void executeProcedure(struct nodeProcedure* p) {
	memory_init();
	if (p->type == 1) {
		executeFuncSeq(p->fs);
	}
	executeDeclSeq(p->ds);
	executeStmtSeq(p->ss);
	cleanUpCrew();
}

void executeFuncSeq(struct nodeFuncSeq* fs) {
	executeFunction(fs->f);
	if (fs->type == 1) {
		executeFuncSeq(fs->fs);
	}
}

void executeFunction(struct nodeFunction* f) {
	functionAdd(f->name, f);
}

void executeDeclSeq(struct nodeDeclSeq* ds) {
	executeDecl(ds->d);
	if (ds->type == 1) {
		executeDeclSeq(ds->ds);
	}
}

void executeDecl(struct nodeDecl* d) {
	if (d->type == 0) {
		executeDeclInt(d->di);
	} else {
		executeDeclRec(d->dr);
	}
}

void executeDeclInt(struct nodeDeclInt* di) {
	declare(di->name, INTEGER);
}

void executeDeclRec(struct nodeDeclRec* dr) {
    declare(dr->name, RECORD);
}

void executeStmtSeq(struct nodeStmtSeq* ss) {
    executeStmt(ss->s);
	if (ss->more == 1) {
		executeStmtSeq(ss->ss);
	}
}

void executeStmt(struct nodeStmt* s) {
	if (s->type == 0) {
		executeAssign(s->assign);
	} else if (s->type == 1) {
		executeIf(s->ifStmt);
	} else if (s->type == 2) {
		executeLoop(s->loop);
	} else if (s->type == 3) {
		executeOut(s->out);
	} else {
		executeCall(s->call);
	}
}

void executeCall(struct nodeCall* c) {
	pushExecutePop(c->name, c->params);
}

void executeAssign(struct nodeAssign* a) {
	if (a->type == 0) {
		int rhs = executeExpr(a->expr);
		store(a->lhs, rhs);
	} else if (a->type == 1) {
		int index = executeIndex(a->index);
		int rhs = executeExpr(a->expr);
		storeRec(a->lhs, index, rhs);
	} else if (a->type == 2) {
		int rhs = executeExpr(a->expr);
		allocateRecord(a->lhs, rhs);
	} else if (a->type == 3) {
		record(a->lhs, a->rhs);
	}
}

int executeIndex(struct nodeIndex* index) {
	return executeExpr(index->expr);
}

void executeOut(struct nodeOut* out) {
	int value = executeExpr(out->expr);
	printf("%d\n", value);
}

void executeIf(struct nodeIf* ifStmt) {
	if (executeCond(ifStmt->cond)) {
		executeStmtSeq(ifStmt->ss1);
	} else if (ifStmt->type == 1) {
		executeStmtSeq(ifStmt->ss2);
	}
}

void executeLoop(struct nodeLoop* loop) {
	while (executeCond(loop->cond)) {
		executeStmtSeq(loop->ss);
	}
}

int executeCond(struct nodeCond* cond) {
	int result = 0;
	if (cond->type == 0) {
		result = executeCmpr(cond->cmpr);
	} else if (cond->type == 1) {
		result = !executeCond(cond->cond);
	} else if (cond->type == 2) {
		result = executeCmpr(cond->cmpr) || executeCond(cond->cond);
	} else if (cond->type == 3) {
		result = executeCmpr(cond->cmpr) && executeCond(cond->cond);
	}
	return result;
}

int executeCmpr(struct nodeCmpr* cmpr) {
	int lhs = executeExpr(cmpr->expr1);
	int rhs = executeExpr(cmpr->expr2);
	return (cmpr->type == 0) ? (lhs == rhs) : (lhs < rhs);
}

int executeExpr(struct nodeExpr* expr) {
	int value = executeTerm(expr->term);
	if (expr->type == 1) {
		value += executeExpr(expr->expr);
	} else if (expr->type == 2) {
		value -= executeExpr(expr->expr);
	}
	return value;
}

int executeTerm(struct nodeTerm* term) {
	int value = executeFactor(term->factor);
	if (term->type == 1) {
		value *= executeTerm(term->term);
	} else if (term->type == 2) {
		int denom = executeTerm(term->term);
		if (denom != 0) {
			value /= denom;
		} else {
			printf("Runtime Error: Division by zero!\n");
			exit(0);
		}
	}
	return value;
}

int executeFactor(struct nodeFactor* factor) {
	int value = 0;
	if (factor->type == 0) {
		value = recall(factor->id);
	} else if (factor->type == 1) {
		int index = executeExpr(factor->expr);
		value = recallRec(factor->id, index);
	} else if (factor->type == 2) {
		value = factor->constant;
	} else if (factor->type == 3) {
		value = executeExpr(factor->expr);
	} else {
		if (currentToken() == EOS) {
			printf("Runtime Error: Data file used up!");
			exit(0);
		}
		value = getConst();
		nextToken();
	}
	return value;
}