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

#ifndef _QAUDIOPLAYER_H_
#define _QAUDIOPLAYER_H_

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <kmainwindow.h>

class QAudioDg2;

/**
 * @short Application Main Window
 * @author Albert Graef <ag@muwiinfa.geschichte.uni-mainz.de>
 * @version 0.3
 */
class QAudioPlayer : public KMainWindow
{
    Q_OBJECT
public:
    /**
     * Default Constructor
     */
    QAudioPlayer();

    /**
     * Default Destructor
     */
    virtual ~QAudioPlayer();
    void startup();
    void startup(const char *arg);
protected:
    QAudioDg2 *view;
};

#endif // _QAUDIOPLAYER_H_
