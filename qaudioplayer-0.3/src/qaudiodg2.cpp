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


#include <kglobal.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kapplication.h>
#include <kstandarddirs.h>
#include <kmainwindow.h>
#include <kmessagebox.h>
#include <kfiledialog.h>
#include <qtimer.h>
#include <qlabel.h>
#include <qslider.h>

#include "qaudiodg2.h"

QAudioDg2::QAudioDg2(QWidget* parent, const char* name, WFlags fl)
    : QAudioDg(parent,name,fl)
{
  // set the application icon
  KIconLoader *loader = KGlobal::iconLoader();
  QPixmap icon;
  icon = loader->loadIcon("qaudioplayer", KIcon::User);
  setIcon(icon);
  // intialize the Q interpreter
#if 1
  QString respath = kapp->dirs()->findResource("data", "qaudioplayer/qaudioplayer.q");
#else
  QString respath = "/home/ag/projects/qaudioplayer/src/qaudioplayer.q";
#endif
  char *path = strdup(respath.latin1());
  interp = new QInterp(path);
  if (!interp->ready) {
    KMessageBox::error(this, i18n("Error loading the qaudioplayer.q script, please check installation (exiting)."));
    exit(1);
  }
  // initialize the timer
  last = -1;
  timer = new QTimer(this);
  connect(timer, SIGNAL(timeout()), SLOT(process()));
  timer->start(100, FALSE);
}

QAudioDg2::~QAudioDg2()
{
}

void QAudioDg2::startup()
{
  QExpr x = call("audio_setup");
  QExprList xs;
  char *driver, *device;
  long rate;
  if (x.istuple(xs) && xs.size() == 3 &&
      xs[0].isstr(driver) && xs[1].isstr(device) &&
      xs[2].isint(rate)) {
    QString msg = QString("%1 (%2) %3KHz").arg(device).arg(driver).arg(rate/1000.0);
    status->setText(msg);
  } else {
    KMessageBox::error(this, i18n("Could not open audio device, please check your audio setup (exiting)."));
    exit(1);
  }
  long win = (long)frame->winId();
  unsigned long fore = frame->foregroundColor().rgb();
  unsigned long back = frame->backgroundColor().rgb();
  call("initcb", win, fore, back);
}

void QAudioDg2::startup(const char *arg)
{
  startup();
  kapp->processEvents(10);
  sleep(1);
  if (open(arg))
    call("playcb");
}

bool QAudioDg2::open(const char *name)
{
  QExpr x = call("sf_ping", name);
  QExprList xs;
  char *basename, *format, *subtype;
  long rate, channels;
  if (x.istuple(xs) && xs.size() == 5 &&
      xs[0].isstr(basename) &&
      xs[1].isint(rate) &&
      xs[2].isint(channels) && channels == 2 &&
      xs[3].isstr(format) &&
      xs[4].isstr(subtype)) {
    QString msg = QString(i18n("%1 %2KHz %3 %4")).arg(basename).arg(rate/1000.0).arg(format).arg(subtype);
    status->setText(msg);
    call("opencb", name);
    return true;
  } else {
    KMessageBox::error(this, i18n("Error opening audio file."));
    repaint();
    return false;
  }
}

/*$SPECIALIZATION$*/
void QAudioDg2::slidercb(int i)
{
  if (i == last) return;
  double pos = (double)i/100000;
  call("slidercb", pos);
}

void QAudioDg2::quitcb()
{
  KMainWindow *main = (KMainWindow*)parent();
  call("finicb");
  main->close();
}

void QAudioDg2::stopcb()
{
  call("stopcb");
}

void QAudioDg2::playcb()
{
  call("playcb");
}

void QAudioDg2::opencb()
{
  QString name = KFileDialog::getOpenFileName(QString::null,
    i18n("*.aiff *.au *.snd *.wav|Audio Files (*.aiff, *.au, *.snd, *.wav)\n*|All files"), this, i18n("Open audio file"));
  if (!name.isEmpty())
   open(name.latin1());
}

void QAudioDg2::fftcb(bool on)
{
  call("fftcb", on);
}

void QAudioDg2::fmincb(int val)
{
  call("fmincb", (long)val);
}

void QAudioDg2::fmaxcb(int val)
{
  call("fmaxcb", (long)val);
}

void QAudioDg2::scalecb(int val)
{
  call("scalecb", (long)val);
}

void QAudioDg2::volumecb(int val)
{
  call("volumecb", (long)val);
}

void QAudioDg2::speedcb(int val)
{
  call("speedcb", (long)val);
}

QExpr QAudioDg2::call(const char *f)
{
  return interp->call(f);
}

QExpr QAudioDg2::call(const char *f, QExpr x)
{
  return interp->call(f, x);
}

QExpr QAudioDg2::call(const char *f, QExpr x, QExpr y)
{
  return interp->call(f, x, y);
}

QExpr QAudioDg2::call(const char *f, QExpr x, QExpr y, QExpr z)
{
  return interp->call(f, x, y, z);
}

QExpr QAudioDg2::tryvar(const char *v)
{
  return call("try", QSym(v));
}

void QAudioDg2::resizeEvent(QResizeEvent *)
{
  long wd = frame->width(), ht = frame->height();
  kapp->processEvents(100);
  usleep(100000);
  call("resizecb", wd, ht);
}

void QAudioDg2::paintEvent(QPaintEvent *)
{
  call("redrawcb");
}

void QAudioDg2::process()
{
  static bool recursive = false;
  if (recursive) return;
  recursive = true;
  while (true) {
    QExpr x = tryvar("MSGS");
    QExprList xs;
    double pos;
    long t;
    if (x.isint(t)) {
      int sec = t%60, min = t/60;
      QString s;
      s.sprintf("%3d:%02d", min, sec);
      time->setText(s);
    } else if (x.istuple(xs) && xs.size() == 2 &&
        xs[0].isfloat(pos) &&
        xs[1].isint(t)) {
      int i = (int)(pos*100000), sec = t%60, min = t/60;
      QString s;
      s.sprintf("%3d:%02d", min, sec);
      last = i;
      slider->setValue(i);
      time->setText(s);
    } else
      break;
  }
  recursive = false;
}

#include "qaudiodg2.moc"

