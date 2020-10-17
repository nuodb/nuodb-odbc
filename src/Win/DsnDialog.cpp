/*
 *     The contents of this file are subject to the Initial
 *     Developer's Public License Version 1.0 (the "License");
 *     you may not use this file except in compliance with the
 *     License. You may obtain a copy of the License at
 *     http://www.ibphoenix.com/main.nfs?a=ibphoenix&page=ibp_idpl.
 *
 *     Software distributed under the License is distributed on
 *     an "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, either
 *     express or implied.  See the License for the specific
 *     language governing rights and limitations under the License.
 *
 *
 *  The Original Code was created by James A. Starkey for IBPhoenix.
 *
 *  Copyright (c) 1999, 2000, 2001 James A. Starkey  All Rights Reserved.
 *  (C) Copyright 2011-2020 NuoDB, Inc.  All Rights Reserved.
 */

#include <stdio.h>
#include <string.h>

#include "OdbcBase.h"

#include <commdlg.h>
#include <cderr.h>

#include <string>
#include <sstream>

#include "../SetupAttributes.h"
#include "OdbcSetup.h"
#include "CommonUtil.h"
#include "DsnDialog.h"
#include "ServiceClient.h"

#include "SQLException.h"

#ifdef IDC_STATIC
# undef IDC_STATIC
#endif
#define IDC_STATIC (-1)

extern HINSTANCE    m_hInstance;
int                 currentCP;

TranslateString translate[] = {
#include "res/resource.en"
    ,
#include "res/resource.ru"
    ,
#include "res/resource.uk"
    ,
#include "res/resource.es"
    ,
#include "res/resource.it"
};

int selectUserLCID(int userLCID)
{
    switch (userLCID) {
        case 0x080a: //     esmx      // Spanish(Mexican)               //
        case 0x0c0a: //     es        // Spanish(Spain - Modern Sort)   //
        case 0x100a: //     esgt      // Spanish(Guatemala)             //
        case 0x140a: //     escr      // Spanish(Costa Rica)            //
        case 0x180a: //     espa      // Spanish(Panama)                //
        case 0x1c0a: //     esdo      // Spanish(Dominican Republic)    //
        case 0x200a: //     esve      // Spanish(Venezuela)             //
        case 0x240a: //     esco      // Spanish(Colombia)              //
        case 0x280a: //     espe      // Spanish(Peru)                  //
        case 0x2c0a: //     esar      // Spanish(Argentina)             //
        case 0x300a: //     esec      // Spanish(Ecuador)               //
        case 0x340a: //     escl      // Spanish(Chile)                 //
        case 0x380a: //     esuy      // Spanish(Uruguay)               //
        case 0x3c0a: //     espy      // Spanish(Paraguay)              //
        case 0x400a: //     esbo      // Spanish(Bolivia)               //
        case 0x440a: //     essv      // Spanish(El Salvador)           //
        case 0x480a: //     eshn      // Spanish(Honduras)              //
        case 0x4c0a: //     esni      // Spanish(Nicaragua)             //
        case 0x500a: //     espr      // Spanish(Puerto Rico)           //
            return 0x040a; // es  // Spanish(Spain-Traditional Sort)//

        case 0x0810: //     itch      // Italian(Swiss)                 //
            return 0x0410; // it  // Italian(Standard)              //
    }

    return userLCID;
}

void initCodePageTranslate(int userLCID)
{
    int i;
    int count = sizeof(translate) / sizeof(*translate);

    userLCID = selectUserLCID(userLCID);

    for (currentCP = -1, i = 0; i < count; i++) {
        if (translate[i].userLCID == userLCID) {
            currentCP = i;
            break;
        }
    }
}

HINSTANCE instanceHtmlHelp = NULL;

INT_PTR CALLBACK    wndprocDsnDialog(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

CDsnDialog::CDsnDialog(HWND hDlgParent)
{
    m_hWndDlg = NULL;
    m_hWndParent = hDlgParent;
    hwndHtmlHelp = NULL;
}

CDsnDialog::~CDsnDialog()
{
    if (instanceHtmlHelp && hwndHtmlHelp) {
        PostMessage(hwndHtmlHelp, WM_DESTROY, (WPARAM)0, (LPARAM)0);
    }
}

void CDsnDialog::SetDisabledDlgItem(HWND hDlg, int ID, BOOL bDisabled)
{
    HWND    hWnd = GetDlgItem(hDlg, ID);
    int     style = GetWindowLong(hWnd, GWL_STYLE);
    if (bDisabled) {
        style |= WS_DISABLED;
    } else {
        style &= ~WS_DISABLED;
    }
    SetWindowLong(hWnd, GWL_STYLE, style);
    InvalidateRect(hWnd, NULL, TRUE);
}

void CDsnDialog::UpdateData(HWND hDlg, BOOL bSaveAndValidate)
{
    if (bSaveAndValidate) {
        char buf[256];
        GetDlgItemText(hDlg, IDC_DATABASE, buf, 256);
        m_database = buf;
        GetDlgItemText(hDlg, IDC_NAME, buf, 256);
        m_name = buf;
        GetDlgItemText(hDlg, IDC_DESCRIPTION, buf, 256);
        m_description = buf;
        GetDlgItemText(hDlg, IDC_PASSWORD, buf, 256);
        m_password = buf;
        GetDlgItemText(hDlg, IDC_USER, buf, 256);
        m_user = buf;
        GetDlgItemText(hDlg, IDC_SCHEMA, buf, 256);
        m_schema = buf;
    } else {
        SetDlgItemText(hDlg, IDC_DATABASE, m_database.c_str());
        SetDlgItemText(hDlg, IDC_NAME, m_name.c_str());
        SetDlgItemText(hDlg, IDC_DESCRIPTION, m_description.c_str());
        SetDlgItemText(hDlg, IDC_PASSWORD, m_password.c_str());
        SetDlgItemText(hDlg, IDC_USER, m_user.c_str());
        SetDlgItemText(hDlg, IDC_SCHEMA, m_schema.c_str());
    }
}

/////////////////////////////////////////////////////////////////////////////
// CDsnDialog message handlers

BOOL CDsnDialog::OnInitDialog(HWND hDlg)
{
    return TRUE;
}

#ifndef _WIN64
# define DWORD_PTR DWORD
#endif

void CDsnDialog::WinHtmlHelp(HWND hDlg)
{
# ifdef UNICODE
#  define HTMLHELP_PROC "HtmlHelpW"
# else
#  define HTMLHELP_PROC "HtmlHelpA"
# endif

    if (!instanceHtmlHelp) {
        instanceHtmlHelp = LoadLibrary("hhctrl.ocx");

        if (!instanceHtmlHelp) {
            return;
        }
    }

    typedef HWND (WINAPI *HtmlHelpProc)(HWND hwndCaller, LPCSTR pszFile, UINT uCommand, DWORD_PTR dwData);

    HtmlHelpProc fn = (HtmlHelpProc)GetProcAddress(instanceHtmlHelp, HTMLHELP_PROC);

    if (!fn) {
        FreeLibrary(instanceHtmlHelp);
        instanceHtmlHelp = NULL;
        return;
    }

    char fileName[512];

    GetModuleFileName(m_hInstance, fileName, sizeof(fileName));

    char* tail = strrchr(fileName, '\\') + 1;
    sprintf(tail, "%s.chm", DRIVER_NAME);

    hwndHtmlHelp = fn(hDlg, (LPCSTR)fileName, 0, 0);
}

INT_PTR CALLBACK wndprocDsnDialog(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    CDsnDialog* dsnDialog = (CDsnDialog*)GetWindowLongPtr(hDlg, GW_USERDATA);

    switch (message) {
        case WM_INITDIALOG:

            SetWindowLongPtr(hDlg, GW_USERDATA, (LONG_PTR)lParam);
            if (!((CDsnDialog*)lParam)->OnInitDialog(hDlg)) {
                return FALSE;
            }
            ((CDsnDialog*)lParam)->UpdateData(hDlg, FALSE);
            return TRUE;

        case WM_COMMAND:
            switch (LOWORD(wParam)) {
                case IDCANCEL:
                    EndDialog(hDlg, FALSE);
                    return TRUE;

                case IDC_TEST_CONNECTION:
                    dsnDialog->UpdateData(hDlg);
                    dsnDialog->OnTestConnection(hDlg);
                    break;

                case IDC_HELP_ODBC:
                    dsnDialog->WinHtmlHelp(hDlg);
                    break;

                case IDOK:
                    dsnDialog->UpdateData(hDlg);
                    EndDialog(hDlg, TRUE);
                    return TRUE;
            }
            break;
    }
    return FALSE;
}

void CDsnDialog::removeNameFileDBfromMessage(char* message)
{
    char* pt = message;

    while ((pt = strstr(pt, "for file"))) {
        while (*pt && *pt != '"') {
            ++pt;
        }
        if (*pt && *pt == '"') {
            char* beg = pt++;

            while (*pt && *pt != '"') {
                ++pt;
            }
            if (*pt && *pt == '"') {
                ++pt;
                *beg++ = 'D';
                *beg++ = 'B';
                memmove(beg, pt, strlen(pt) + 1);
            }
        }
    }
}

void CDsnDialog::OnTestConnection(HWND hDlg)
{
    char strHeadDlg[256];

    GetWindowText(hDlg, strHeadDlg, sizeof(strHeadDlg));

    try {
        CServiceClient services;

        UpdateData(hDlg);

        if (!services.initServices()) {
            std::ostringstream text;
            text << "Unable to connect to data source: library '" << DRIVER_NAME << "' failed to load";
            MessageBox(hDlg, text.str().c_str(), TEXT(strHeadDlg), MB_ICONERROR|MB_OK);
            return;
        }

        if (!services.checkVersion()) {
            std::ostringstream text;
            text << "Unable to connect to load " << DRIVER_NAME << " Library : can't find ver. " << DRIVER_VERSION;
            MessageBox(hDlg, text.str().c_str(), "Connection failed!", MB_ICONINFORMATION|MB_OK);
            return;
        }

        if (!m_user.empty()) {
            services.putParameterValue("user", m_user);
        }
        if (!m_password.empty()) {
            services.putParameterValue("password", m_password);
        }
        if (!m_schema.empty()) {
            services.putParameterValue("schema", m_schema);
        }

        if (!m_database.empty()) {
            services.putParameterValue(SETUP_DBNAME, m_database);
        }

        std::string openError;
        if (m_database.empty() || !services.openDatabase(&openError)) {
            std::ostringstream text;
            text << "Open database '" << m_database << "' failed: " << openError;
            MessageBox(hDlg, text.str().c_str(), TEXT(strHeadDlg), MB_ICONERROR | MB_OK);
            return;
        }

        MessageBox(hDlg, "Connection successful!", TEXT(strHeadDlg), MB_ICONINFORMATION | MB_OK);
    } catch (NuoDB::SQLException& exception) {
        char buffer[2048];
        const char* text = exception.getText();

        sprintf(buffer, "%s\n%s", "Connection failed!", text);
        removeNameFileDBfromMessage(buffer);

        MessageBox(hDlg, TEXT(buffer), TEXT(strHeadDlg), MB_ICONERROR | MB_OK);
    }
}

intptr_t CDsnDialog::DoModal()
{
    WORD*   p, * pdlgtemplate;
    int     nchar;
    DWORD   lStyle;

    pdlgtemplate = p = (PWORD)LocalAlloc(LPTR, 4096);
    lStyle = DS_SETFONT | DS_MODALFRAME | DS_CENTER | WS_POPUP | WS_CAPTION | WS_SYSMENU | WS_VISIBLE;

    *p++ = LOWORD(lStyle);
    *p++ = HIWORD(lStyle);
    *p++ = 0;       // LOWORD (lExtendedStyle)
    *p++ = 0;       // HIWORD (lExtendedStyle)

    *p++ = 16;      // NumberOfItems

    *p++ = 0;       // x
    *p++ = 0;       // y
    *p++ = 310;     // cx
    *p++ = 252;     // cy
    *p++ = 0;       // Menu
    *p++ = 0;       // Class

    // copy the title of the dialog
    nchar = nCopyAnsiToWideChar(p, TEXT("NuoDB ODBC Setup"));
    p += nchar;

    *p++ = 8;       // FontSize
    nchar = nCopyAnsiToWideChar(p, TEXT("MS Sans Serif"));
    p += nchar;

    TMP_EDITTEXT(IDC_NAME, 7, 12, 184, 12, ES_AUTOHSCROLL)
    TMP_LTEXT(SETUP_DESCRIPTION, IDC_STATIC, 7, 26, 218, 8)
    TMP_EDITTEXT(IDC_DESCRIPTION, 7, 36, 296, 12, ES_AUTOHSCROLL)
    TMP_EDITTEXT(IDC_DATABASE, 7, 61, 231, 12, ES_AUTOHSCROLL)
    TMP_EDITTEXT(IDC_USER, 7, 85, 107, 12, ES_AUTOHSCROLL)
    TMP_EDITTEXT(IDC_PASSWORD, 118, 85, 74, 12, ES_PASSWORD | ES_AUTOHSCROLL)
    TMP_EDITTEXT(IDC_SCHEMA, 7, 110, 107, 12, ES_AUTOHSCROLL)
    TMP_DEFPUSHBUTTON("OK", IDOK, 86, 233, 60, 14)
    TMP_PUSHBUTTON("Cancel", IDCANCEL, 154, 233, 60, 14)
    TMP_LTEXT("Data Source Name (DSN)", IDC_STATIC, 7, 2, 167, 8)
    TMP_LTEXT(SETUP_DATABASE, IDC_STATIC, 7, 51, 218, 8)
    TMP_LTEXT(SETUP_USER, IDC_STATIC, 7, 75, 107, 8)
    TMP_LTEXT(SETUP_PASSWORD, IDC_STATIC, 119, 75, 72, 8)
    TMP_LTEXT(SETUP_SCHEMA, IDC_STATIC, 7, 100, 218, 8)
    TMP_PUSHBUTTON("Test connection", IDC_TEST_CONNECTION, 216, 130, 87, 18)
    TMP_PUSHBUTTON("Help", IDC_HELP_ODBC, 243, 233, 60, 14)

    intptr_t nRet = DialogBoxIndirectParam(m_hInstance, (LPDLGTEMPLATE)pdlgtemplate, m_hWndParent, wndprocDsnDialog, (LPARAM) this);
    LocalFree(LocalHandle(pdlgtemplate));

    return nRet;
}
