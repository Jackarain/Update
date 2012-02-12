#pragma once

// Set max target Windows platform
#define WINVER 0x0500
#define _WIN32_WINNT 0x0501

// Use target Common Controls version for compatibility
// with CPropertyPageEx, CPropertySheetEx
#if _WIN32_IE < 0x0500
#define _WIN32_IE 0x0500
#endif

#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers

#ifndef WS_EX_LAYOUTRTL
#pragma message("Please update your Windows header files, get the latest SDK")
#pragma message("WinUser.h is out of date!")

#define WS_EX_LAYOUTRTL		0x00400000
#endif

#ifndef WC_BUTTON
#pragma message("Please update your Windows header files, get the latest SDK")
#pragma message("CommCtrl.h is out of date!")

#define WC_BUTTON			TEXT("Button")
#define WC_STATIC			TEXT("Static")
#define WC_EDIT				TEXT("Edit")
#define WC_LISTBOX			TEXT("ListBox")
#define WC_COMBOBOX			TEXT("ComboBox")
#define WC_SCROLLBAR		TEXT("ScrollBar")
#endif

#include <atlcrack.h>

#define RSZLIB_NO_XP_DOUBLE_BUFFER

#include <RegisteredMsg.h>
#include <ResizableMsgSupport.h>
#include <ResizableState.h>
#include <ResizableGrip.h>
#include <ResizableLayout.h>
#include <ResizableVersion.h>
#include <ResizableMinMax.h>
#include <ResizableDialog.h>
#include <ResizableSheet.h>
#include <ResizablePage.h>

