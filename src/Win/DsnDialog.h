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

#pragma once

#include <string>

#include "OdbcBase.h"

struct TranslateString
{
    int userLCID;
    struct
    {
        int id;
        char* string;
    } table[64];
};

#define _TR(id, msg) (currentCP == -1 ? msg : translate[currentCP].table[id].string)

class CServiceTabCtrl;

class CDsnDialog
{
    HWND hwndHtmlHelp;

//  const char** drivers;
//  const char** charsets;
//  const char** useshemas;

public:
    CDsnDialog(HWND hDlgParent);
    ~CDsnDialog();

// Dialog Data
    enum { IDD = IDD_DSN_PROPERTIES };
    HWND    m_hWndParent;
    HWND    m_hWndDlg;
    std::string  m_database;
    std::string  m_name;
    std::string  m_description;
    std::string  m_password;
    std::string  m_user;
    std::string  m_schema;

public:
    intptr_t    DoModal();
    void        SetDisabledDlgItem(HWND hDlg, int ID, BOOL bDisabled=TRUE);
    void        UpdateData(HWND hDlg, BOOL bSaveAndValidate=TRUE);
    BOOL        OnInitDialog(HWND hDlg);
#ifdef _WINDOWS
    void    OnTestConnection(HWND hDlg);
    void    WinHtmlHelp(HWND hDlg);
#endif
    void removeNameFileDBfromMessage(char* message);
};
