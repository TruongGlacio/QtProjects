/***************************************************************************
 *   Copyright (C) 2004 by Albert Graef                                    *
 *   ag@muwiinfa.geschichte.uni-mainz.de                                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include "qinterp.h"

QSym::QSym(const char *s) { qacquire(); sym = qgetsym(s); qrelease(); }

QSym::QSym(int s) : sym(s) { }

QExpr::QExpr() : expr(NULL) { }

QExpr::QExpr(qexpr x) { qacquire(); expr = qnewref(x); qrelease(); }

QExpr::QExpr(bool x) { qacquire(); expr = qnewref(qmkbool(x)); qrelease(); }

QExpr::QExpr(long x) { qacquire(); expr = qnewref(qmkint(x)); qrelease(); }

QExpr::QExpr(unsigned long x) { qacquire(); expr = qnewref(qmkuint(x)); qrelease(); }

QExpr::QExpr(double x) { qacquire(); expr = qnewref(qmkfloat(x)); qrelease(); }

QExpr::QExpr(const char *x) { qacquire(); expr = qnewref(qmkstr(strdup(x))); qrelease(); }

QExpr::QExpr(QSym x) { qacquire(); expr = qnewref(qmksym(x.sym)); qrelease(); }
  
QExpr::~QExpr() { qacquire(); qfreeref(expr); qrelease(); }

QExpr::QExpr(const QExpr &x) { qacquire(); expr = qnewref(x.expr); qrelease(); }
  
QExpr& QExpr::operator=(const QExpr &x)
{
  if (this != &x) {
    qacquire();
    qfreeref(expr);
    expr = qnewref(x.expr);
    qrelease();
  }
  return *this;
}

QExpr QAppExpr(QExpr x, QExpr y)
{
  qacquire();
  qexpr ret = qmkapp(x.expr, y.expr);
  qrelease();
  return ret;
}

QExpr QConsExpr(QExpr x, QExpr y)
{
  qacquire();
  qexpr ret = qmkcons(x.expr, y.expr);
  qrelease();
  return ret;
}

QExpr QContExpr(QExpr x, QExpr y)
{
  qacquire();
  qexpr ret = qmkcont(x.expr, y.expr);
  qrelease();
  return ret;
}

QExpr QListExpr(QExprList &xs)
{
  int n = xs.size();
  qexpr *v = (qexpr*)malloc(n*sizeof(qexpr));
  if (v) {
    int i;
    QExprList::iterator it;
    for (i = 0, it = xs.begin(); i < n && it != xs.end(); i++, it++)
      v[i] = (*it).expr;
    qacquire();
    qexpr ret = qmklistv(n, v);
    qrelease();
    return ret;
  } else
    return QExpr();
}

QExpr QTupleExpr(QExprList &xs)
{
  int n = xs.size();
  qexpr *v = (qexpr*)malloc(n*sizeof(qexpr));
  if (v) {
    int i;
    QExprList::iterator it;
    for (i = 0, it = xs.begin(); i < n && it != xs.end(); i++, it++)
      v[i] = (*it).expr;
    qacquire();
    qexpr ret = qmktuplev(n, v);
    qrelease();
    return ret;
  } else
    return QExpr();
}

bool QExpr::isnull()
{
  return expr == NULL;
}

bool QExpr::isnil()
{
  return qisnil(expr);
}

bool QExpr::isvoid()
{
  return qisvoid(expr);
}

bool QExpr::istrue()
{
  return qistrue(expr);
}

bool QExpr::isfalse()
{
  return qisfalse(expr);
}

bool QExpr::isbool(bool &x)
{
  int res;
  if (qisbool(expr, &res)) {
    x = res != 0;
    return true;
  } else
    return false;
}

bool QExpr::isint(long &x)
{
  return qisint(expr, &x);
}

bool QExpr::isuint(unsigned long &x)
{
  return qisuint(expr, &x);
}

bool QExpr::isfloat(double &x)
{
  return qisfloat(expr, &x);
}

bool QExpr::isstr(char* &x)
{
  return qisstr(expr, &x);
}

bool QExpr::issym(int sym)
{
  return qissym(expr, sym);
}

bool QExpr::isapp(QExpr &x, QExpr &y)
{
  qexpr a, b;
  if (qisapp(expr, &a, &b)) {
    x = QExpr(a);
    y = QExpr(b);
    return true;
  } else
    return false;
}

bool QExpr::iscons(QExpr &x, QExpr &y)
{
  qexpr a, b;
  if (qiscons(expr, &a, &b)) {
    x = QExpr(a);
    y = QExpr(b);
    return true;
  } else
    return false;
}

bool QExpr::iscont(QExpr &x, QExpr &y)
{
  qexpr a, b;
  if (qiscont(expr, &a, &b)) {
    x = QExpr(a);
    y = QExpr(b);
    return true;
  } else
    return false;
}

bool QExpr::islist(QExprList &xs)
{
  qexpr x = expr, hd, tl;
  while (qiscons(x, &hd, &tl))
    x = tl;
  if (!qisnil(x))
    return false;
  x = expr;
  xs = QExprList();
  while (qiscons(x, &hd, &tl)) {
    xs.append(QExpr(hd));
    x = tl;
  }
  return true;
}

bool QExpr::istuple(QExprList &xs)
{
  int i, n;
  qexpr *v;
  if (qistuple(expr, &n, &v)) {
    xs = QExprList();
    for (i = 0; i < n; i++)
      xs.append(QExpr(v[i]));
    return true;
  } else
    return false;
}

QInterp *QInterp::qinterp = 0;

QInterp::QInterp()
{
  if (qinterp) qacquire();
  status = qexecl(NULL, 0);
  ready = status == 0;
  qinterp = this;
  qrelease();
}

QInterp::QInterp(const char *path)
{
  if (qinterp) qacquire();
  status = qexecv(path, 0, NULL);
  ready = status == 0;
  qinterp = this;
  qrelease();
}

QInterp::QInterp(const char *path, int argc, char *const argv[])
{
  if (qinterp) qacquire();
  status = qexecv(path, argc, argv);
  ready = status == 0;
  qinterp = this;
  qrelease();
}

QInterp::~QInterp()
{
  qacquire();
  qexecl(NULL, 0);
  qinterp = 0;
}

const char *QInterp::strerror(int status)
{
  return qstrerror(status);
}

QExpr QInterp::eval(QExpr x)
{
  qacquire();
  qexpr ret = qevalx(x.expr, &status);
  qrelease();
  return ret;
}

bool QInterp::parse(const QCString &s, QExpr &x)
{
  qacquire();
  qexpr y = qparse((const char*)s, &status);
  qrelease();
  if (y) {
    x = QExpr(y);
    return true;
  } else
    return false;
}

bool QInterp::print(QExpr x, QCString &s)
{
  qacquire();
  char *t = qprint(x.expr, &status);
  qrelease();
  if (t) {
    s = t;
    free(t);
    return true;
  } else
    return false;
}
 
int QInterp::init_thread()
{
  int res = qinit_thread();
  qrelease();
  return res;
}
 
void QInterp::fini_thread(int id)
{
  qacquire();
  qfini_thread(id);
}
