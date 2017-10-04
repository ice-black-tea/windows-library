#pragma once

#include <Windows.h>
#include <assert.h>
#include "../Common.h"

class CDebugger;

const int MAX_BIN_CODE = 0x20; //一条二进制指令最大长度（暂定）
const int MAX_ASM_CODE = 0x40; //一条汇编指令字符数（暂定）


//调试器回调类型
typedef enum emDEBUG_NOTIFY_TYPE
{
    NOTIFY_DEBUGEVENT_PRE,
    NOTIFY_DEBUGEVENT_POST,
    NOTIFY_PROCESS_CREATE,
    NOTIFY_PROCESS_EXIT,
    NOTIFY_BREAKPOINT,
    NOTIFY_SYSTEM_BREAKPOINT,
    NOTIFY_ERROR
}DEBUG_NOTIFY_TYPE;


//调试事件
class CDebugEvent : public DEBUG_EVENT
{
public:
    CDebugEvent() { memset(this, 0, sizeof(CDebugEvent)); }
    ~CDebugEvent() { }

public:
    DWORD m_dwContinueStatus;
};


//调试回调函数
typedef void(*PFNDebugNotify)(DEBUG_NOTIFY_TYPE type, CDebugger *pDebugger, CDebugEvent *pDbgEvt);


//switch...case...根据调试事件调用响应函数
#define BEGIN_DEBUG_EVENT_MAP() static DWORD OnDebugEvent(DWORD dwEvtCode, CDebugger *pDebugger, CDebugEvent *pDbgEvt) { switch (dwEvtCode) {
#define DECLARE_DEBUG_EVENT(excpCode, pfnHandle) case excpCode: return pDebugger->pfnHandle(pDbgEvt);
#define END_DEBUG_EVENT_MAP() default: return DBG_EXCEPTION_NOT_HANDLED; } }


//switch...case...根据异常类型调用响应函数
#define BEGIN_EXCEPTION_MAP() DWORD OnExceptionDebugEvent(CDebugEvent *pDbgEvt) { switch (pDbgEvt->u.Exception.ExceptionRecord.ExceptionCode) {
#define DECLARE_EXCEPTION(excpCode, pfnHandle) case excpCode: return pfnHandle(pDbgEvt);
#define END_EXCEPTION_MAP() default: return DBG_EXCEPTION_NOT_HANDLED; } }