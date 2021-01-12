#include "ProcessorInformation.h"


CProcessorInformation::CProcessorInformation()
    :m_dwNumaNodeCount(0),
    m_dwProcessorPackageCount(0),
    m_dwProcessorCoreCount(0),
    m_dwLogicalProcessorCount(0),
    m_dwProcessorL1CacheCount(0),
    m_dwProcessorL2CacheCount(0),
    m_dwProcessorL3CacheCount(0)
{
    BOOL done = FALSE;
    PSYSTEM_LOGICAL_PROCESSOR_INFORMATION buffer = NULL;
    PSYSTEM_LOGICAL_PROCESSOR_INFORMATION ptr = NULL;
    DWORD returnLength = 0;
    DWORD byteOffset = 0;
    PCACHE_DESCRIPTOR Cache;


    while (!done)
    {
        DWORD rc = GetLogicalProcessorInformation(buffer, &returnLength);

        if (FALSE == rc)
        {
            if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
            {
                if (buffer)
                    free(buffer);

                buffer = (PSYSTEM_LOGICAL_PROCESSOR_INFORMATION)malloc(
                    returnLength);

                if (NULL == buffer)
                {
                    return;
                }
            }
            else
            {
                return;
            }
        }
        else
        {
            done = TRUE;
        }
    }

    ptr = buffer;

    while (byteOffset + sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION) <= returnLength)
    {
        switch (ptr->Relationship)
        {
        case RelationNumaNode:
            // Non-NUMA systems report a single record of this type.
            m_dwNumaNodeCount++;
            break;

        case RelationProcessorCore:
            m_dwProcessorCoreCount++;

            // A hyperthreaded core supplies more than one logical processor.
            m_dwLogicalProcessorCount += CountSetBits(ptr->ProcessorMask);
            break;

        case RelationCache:
            // Cache data is in ptr->Cache, one CACHE_DESCRIPTOR structure for each cache. 
            Cache = &ptr->Cache;
            if (Cache->Level == 1)
            {
                m_dwProcessorL1CacheCount++;
            }
            else if (Cache->Level == 2)
            {
                m_dwProcessorL2CacheCount++;
            }
            else if (Cache->Level == 3)
            {
                m_dwProcessorL3CacheCount++;
            }
            break;

        case RelationProcessorPackage:
            // Logical processors share a physical package.
            m_dwProcessorPackageCount++;
            break;

        default:
            break;
        }
        byteOffset += sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION);
        ptr++;
    }

    free(buffer);
}


CProcessorInformation::~CProcessorInformation()
{
}


// Helper function to count set bits in the processor mask.
DWORD CProcessorInformation::CountSetBits(ULONG_PTR bitMask)
{
    DWORD LSHIFT = sizeof(ULONG_PTR) * 8 - 1;
    DWORD bitSetCount = 0;
    ULONG_PTR bitTest = (ULONG_PTR)1 << LSHIFT;
    DWORD i;

    for (i = 0; i <= LSHIFT; ++i)
    {
        bitSetCount += ((bitMask & bitTest) ? 1 : 0);
        bitTest /= 2;
    }

    return bitSetCount;
}
