#ifndef MSGBOX_H
#define MSGBOX_H

#include <string>

const MB_OK = 1;
const MB_CANCEL = 2;
const MB_YES = 4;
const MB_NO = 8;

const MB_OKCANCEL = MB_OK|MB_CANCEL;
const MB_YESNO = MB_YES|MB_NO;

const nBtnTypes = 4;

int MsgBox (const char* title, const char* msg, int btns);

#endif
