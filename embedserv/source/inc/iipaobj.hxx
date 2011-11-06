/**************************************************************
 * 
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 * 
 *   http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 * 
 *************************************************************/



#ifndef _IIPAOBJ_HXX_
#define _IIPAOBJ_HXX_
#if defined(_MSC_VER) && (_MSC_VER > 1310)
#pragma warning(disable : 4917 4555)
#endif

#include "stdafx.h"
#include <oleidl.h>

#include <osl/interlck.h>
#include <rtl/ref.hxx>
class EmbedDocument_Impl;
class DocumentHolder;

class CIIAObj
    : public IOleInPlaceActiveObject
{

public:

	CIIAObj( DocumentHolder * );
    virtual ~CIIAObj();

    /* IUnknown methods */
    STDMETHODIMP QueryInterface(REFIID, LPVOID FAR * ppvObj);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

    /* IOleInPlaceActiveObject methods */
    STDMETHODIMP GetWindow(HWND *);
    STDMETHODIMP ContextSensitiveHelp(BOOL);
    STDMETHODIMP TranslateAccelerator(LPMSG);
    STDMETHODIMP OnFrameWindowActivate(BOOL);
    STDMETHODIMP OnDocWindowActivate(BOOL);
    STDMETHODIMP ResizeBorder(LPCRECT, LPOLEINPLACEUIWINDOW
                              , BOOL);
    STDMETHODIMP EnableModeless(BOOL);


private:

	oslInterlockedCount					m_refCount;
    ::rtl::Reference< DocumentHolder >  m_rDocHolder;
};


#endif