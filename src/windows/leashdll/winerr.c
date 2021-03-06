/*  WINERR.C

    Jason Hunter
    8/2/94
    DCNS/IS MIT


  Contains the error functions for leash and kerberos.  Prints out keen windows
  error messages in english.

*/

#include <stdio.h>
#include "conf.h"

// Private Include files
#include "leashdll.h"
#include <leashwin.h>

// Global Variables.
static long lsh_errno;
static char *err_context;       /* error context */
extern int (*Lcom_err)(LPSTR,long,LPSTR,...);
extern LPSTR (*Lerror_message)(long);
extern LPSTR (*Lerror_table_name)(long);

#ifdef WIN16
#define UNDERSCORE "_"
#else
#define UNDERSCORE
#endif

HWND GetRootParent (HWND Child)
{
    HWND Last;
    while (Child)
    {
        Last = Child;
        Child = GetParent (Child);
    }
    return Last;
}


LPSTR err_describe(LPSTR buf, long code)
{
    LPSTR cp, com_err_msg;
    int offset;
    long table_num;
    char *etype;

    offset = (int) (code & 255);
    table_num = code - offset;
    com_err_msg = Lerror_message(code);

    lstrcpy(buf, com_err_msg);
    return buf;


////Is this needed at all after the return above?
    cp = buf;
    if(com_err_msg != buf)
        lstrcpy(buf, com_err_msg);
    cp = buf + lstrlen(buf);
    *cp++ = '\n';
    etype = Lerror_table_name(table_num);
    wsprintf((LPSTR) cp, (LPSTR) "(%s error %d"
#ifdef DEBUG_COM_ERR
             " (absolute error %ld)"
#endif
             ")", etype, offset
             //")\nPress F1 for help on this error.", etype, offset
#ifdef DEBUG_COM_ERR
             , code
#endif
        );

    return (LPSTR)buf;
}

int _export lsh_com_err_proc (LPSTR whoami, long code,
                              LPSTR fmt, va_list args)
{
#ifdef USE_MESSAGE_BOX
    int retval;
    HWND hOldFocus;
    char buf[1024], *cp; /* changed to 512 by jms 8/23/93 */
    WORD mbformat = MB_OK | MB_ICONEXCLAMATION;

    cp = buf;
    memset(buf, '\0', sizeof(buf));
    cp[0] = '\0';

    if (code)
    {
        err_describe(buf, code);
        while (*cp)
            cp++;
    }

    if (fmt)
    {
        if (fmt[0] == '%' && fmt[1] == 'b')
	{
            fmt += 2;
            mbformat = va_arg(args, WORD);
            /* if the first arg is a %b, we use it for the message
               box MB_??? flags. */
	}
        if (code)
	{
            *cp++ = '\n';
            *cp++ = '\n';
	}
        wvsprintf((LPSTR)cp, fmt, args);
    }
    hOldFocus = GetFocus();
    retval = MessageBox(/*GetRootParent(hOldFocus)*/NULL, buf, whoami,
                        mbformat | MB_ICONHAND | MB_TASKMODAL);
    SetFocus(hOldFocus);
    return retval;
#else
    return IDOK;
#endif /* USE_MESSAGE_BOX */
}
