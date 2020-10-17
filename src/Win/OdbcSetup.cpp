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

#include "OdbcBase.h"

#include <commctrl.h>

#include "OdbcSetup.h"

HINSTANCE m_hInstance = NULL;
void initCodePageTranslate(int userLCID);

BOOL APIENTRY DllMainSetup(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID)
{
    if (fdwReason == DLL_PROCESS_ATTACH) {
        m_hInstance = hinstDLL;
        InitCommonControls();
        initCodePageTranslate(GetUserDefaultLCID());
    }

    return TRUE;
}
