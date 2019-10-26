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

#ifndef QAUDIODG2_H
#define QAUDIODG2_H

#include "qinterp.h"
#include "qaudiodg.h"

class QAudioDg2 : public QAudioDg
{
  Q_OBJECT

public:
  QAudioDg2(QWidget* parent = 0, const char* name = 0, WFlags fl = 0 );
  ~QAudioDg2();
  /*$PUBLIC_FUNCTIONS$*/
  void startup();
  void startup(const char *arg);
  bool open(const char *name);

public slots:
  /*$PUBLIC_SLOTS$*/
  virtual void          slidercb( int );
  virtual void          quitcb();
  virtual void          stopcb();
  virtual void          playcb();
  virtual void          opencb();
  virtual void          fftcb( bool );
  virtual void          fmincb( int );
  virtual void          fmaxcb( int );
  virtual void          scalecb( int );
  virtual void          volumecb( int );
  virtual void          speedcb( int );

protected:
  /*$PROTECTED_FUNCTIONS$*/
  void resizeEvent(QResizeEvent *);
  void paintEvent(QPaintEvent *);

protected slots:
  /*$PROTECTED_SLOTS$*/

protected:
  int last;
  QTimer *timer;
  QInterp *interp;
  QExpr call(const char *f);
  QExpr call(const char *f, QExpr x);
  QExpr call(const char *f, QExpr x, QExpr y);
  QExpr call(const char *f, QExpr x, QExpr y, QExpr z);
  QExpr tryvar(const char *v);

protected slots:
  void process();
};

#endif

