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

#ifndef _QINTERP_H_
#define _QINTERP_H_

#include <qcstring.h>
#include <qvaluelist.h>
#include <qint.h>

/* C++ interface to the Q interpreter. The classes in this module are for
   evaluating expressions in the Q interpreter. Please take a look at qint.h
   (provided with the Q base package) for a closer description of the provided
   operations. */

/* The following class embodies Q expression values. */

class QExpr;

typedef QValueList<QExpr> QExprList;

class QSym {
public:
  QSym(const char *s);
  QSym(int s);
  int sym;
};

class QExpr {
  friend class QInterp;
  friend QExpr QAppExpr(QExpr x, QExpr y);
  friend QExpr QConsExpr(QExpr x, QExpr y);
  friend QExpr QContExpr(QExpr x, QExpr y);
  friend QExpr QListExpr(/*const*/ QExprList &xs);
  friend QExpr QTupleExpr(/*const*/ QExprList &xs);
public:
  QExpr();
  QExpr(qexpr x);
  QExpr(bool x);
  QExpr(long x);
  QExpr(unsigned long x);
  QExpr(double x);
  QExpr(const char *x);
  QExpr(QSym x);
  
  ~QExpr();

  QExpr(const QExpr &x);
  QExpr& operator=(const QExpr &x);
  
  bool isnull();
  bool isnil();
  bool isvoid();
  bool istrue();
  bool isfalse();
  bool isbool(bool &x);
  bool isint(long &x);
  bool isuint(unsigned long &x);
  bool isfloat(double &x);
  bool isstr(char* &x);
  bool issym(int sym);

  bool isapp(QExpr &x, QExpr &y);
  bool iscons(QExpr &x, QExpr &y);
  bool iscont(QExpr &x, QExpr &y);
  bool islist(QExprList &xs);
  bool istuple(QExprList &xs);
  
  qexpr expr;
};

/* Access to the Q interpreter is implemented by the following class.
   Note that currently only a single interpreter instance can be used
   at any one time. */

#define qint QInterp::qinterp

class QInterp {
public:
  // constructors and destructors
  QInterp();
  QInterp(const char *path);
  QInterp(const char *path, int argc, char *const argv[]);
  ~QInterp();

  // error messages
  const char *strerror(int status);
  
  // evaluate an expression
  QExpr eval(QExpr x);
  QExpr call(const char *s)
    { return eval(QSym(s)); };
  QExpr call(const char *s, QExpr x)
    { return eval(QAppExpr(QSym(s), x)); };
  QExpr call(const char *s, QExpr x, QExpr y)
    { return eval(QAppExpr(QAppExpr(QSym(s), x), y)); };
  QExpr call(const char *s, QExpr x, QExpr y, QExpr z)
    { return eval(QAppExpr(QAppExpr(QAppExpr(QSym(s), x), y), z)); };

  // parse and unparse an expression  
  bool parse(const QCString &s, QExpr &x);
  bool print(QExpr x, QCString &s);
  
  // thread management
  int init_thread();
  void fini_thread(int id);
  
  bool ready; // interpreter has been initialized properly
  int status; // status code of initialization/last eval
  
  static QInterp *qinterp; // the global interpreter object
};


#endif // _QINTERP_H_
