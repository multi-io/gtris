/*  $Id: msgbox.h,v 1.2.4.2 1999/10/17 09:30:11 olaf Exp $ */

/*  GTris
 *  $Name:  $
 *  Copyright (C) 1999  Olaf Klischat
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef MSGBOX_H
#define MSGBOX_H

const int MB_OK = 1;
const int MB_CANCEL = 2;
const int MB_YES = 4;
const int MB_NO = 8;

const int MB_OKCANCEL = MB_OK|MB_CANCEL;
const int MB_YESNO = MB_YES|MB_NO;

const int nBtnTypes = 4;

int MsgBox (const char* title, const char* msg, int btns);

#endif
