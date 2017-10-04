#include "BreakPoint.h"


IBreakPoint::IBreakPoint()
{

}


IBreakPoint::~IBreakPoint()
{

};




void CBreakPointList::Set(DWORD dwPid)
{
    iterator itBp = begin();

    for (; itBp != end(); itBp++)
    {
        CBreakPointPtr pBp = *itBp;

        if (pBp->GetStatus() != BREAKPOINT_FORBIDDEN)
        {
            pBp->Set(dwPid);
        }
    }
}


void CBreakPointList::Reset(DWORD dwPid)
{
    iterator itList = begin();

    for (; itList != end(); itList++)
    {
        CBreakPointPtr pBp = *itList;

        if (pBp->IsSet())
        {
            pBp->Reset(dwPid);
        }
    }
}