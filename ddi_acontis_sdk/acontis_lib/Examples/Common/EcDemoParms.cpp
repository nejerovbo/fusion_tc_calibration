/*-----------------------------------------------------------------------------
 * EcDemoParms.cpp
 * Copyright                acontis technologies GmbH, Ravensburg, Germany
 * Response                 Holger Oelhaf
 * Description              Common settings for EC-Master demo
 *---------------------------------------------------------------------------*/

/*-INCLUDES------------------------------------------------------------------*/
#include "EcDemoApp.h"

/*-DEFINES-------------------------------------------------------------------*/
#define LOG_BUFFER_SIZE         ((EC_T_DWORD)0x1000)

/*-GLOBAL VARIABLES----------------------------------------------------------*/
volatile EC_T_BOOL bRun = EC_TRUE;

/*-DEFINES-------------------------------------------------------------------*/
#ifndef BIT2BYTE
    #define BIT2BYTE(x) \
        (((x)+7)>>3)
#endif

/*-FUNCTION-DEFINITIONS------------------------------------------------------*/
EC_T_VOID ResetAppParms(T_EC_DEMO_APP_CONTEXT* pAppContext, T_EC_DEMO_APP_PARMS* pAppParms)
{
    EC_UNREFPARM(pAppContext);

    OsMemset(pAppParms, 0, sizeof(T_EC_DEMO_APP_PARMS));
    pAppParms->eCnfType = eCnfType_GenPreopENI;
    pAppParms->dwBusCycleTimeUsec = 1000;
    pAppParms->dwDemoDuration = 600000;
    pAppParms->bConnectHcGroups = EC_TRUE;
}

EC_T_DWORD SetAppParmsFromCommandLine(T_EC_DEMO_APP_CONTEXT* pAppContext, EC_T_CHAR* szCommandLine, T_EC_DEMO_APP_PARMS* pAppParms)
{
    EC_T_DWORD dwRetVal = EC_E_ERROR;
    EC_T_INT   nVerbose = 1;
    EC_T_BOOL  bGetNextWord = EC_TRUE;
    EC_T_CHAR* ptcWord = EC_NULL;
    EC_T_INT   nPtcWordIdx = 0;
#if (!defined ATEMRAS_CLIENT)
    EC_T_CHAR  tcStorage = '\0';
    EC_T_DWORD dwNumLinkLayer = 0;
#endif
    EC_T_CHAR  szCommandLineTmp[COMMAND_LINE_BUFFER_LENGTH];

    OsStrncpy(szCommandLineTmp, szCommandLine, COMMAND_LINE_BUFFER_LENGTH - 1);

    for (ptcWord = OsStrtok(szCommandLineTmp, " "); ptcWord != EC_NULL;)
    {
        if (0 == OsStricmp(ptcWord, "-a"))
        {
            ptcWord = OsStrtok(NULL, " ");
            if ((ptcWord == EC_NULL) || (OsStrncmp(ptcWord, "-", 1) == 0))
            {
                dwRetVal = EC_E_INVALIDPARM;
                goto Exit;
            }
            pAppParms->dwCpuIndex = OsStrtol(ptcWord, EC_NULL, 0);
            EC_CPUSET_SET(pAppParms->CpuSet, pAppParms->dwCpuIndex);
        }
#if (defined INCLUDE_RAS_SPOCSUPPORT)
        else if (0 == OsStricmp(ptcWord, "-ac"))
        {
            ptcWord = OsStrtok(EC_NULL, " ");
            if ((ptcWord == NULL) || (0 == OsStrncmp(ptcWord, "-", 1)))
            {
                pAppParms->bRasAccessControlEnabled = EC_FALSE;

                /* optional sub parameter not found, use the current word for the next parameter */
                bGetNextWord = EC_FALSE;
            }
            else
            {
                pAppParms->bRasAccessControlEnabled = ((EC_T_WORD)OsStrtol(ptcWord, EC_NULL, 10) == 1);
            }
        }
        else if (0 == OsStricmp(ptcWord, "-acinit"))
        {
            ptcWord = OsStrtok(EC_NULL, " ");
            if ((ptcWord == NULL) || (0 == OsStrncmp(ptcWord, "-", 1)))
            {
                pAppParms->dwRasAccessLevel = ATEMRAS_ACCESS_LEVEL_ALLOW_ALL;

                /* optional sub parameter not found, use the current word for the next parameter */
                bGetNextWord = EC_FALSE;
            }
            else
            {
                pAppParms->dwRasAccessLevel = (EC_T_DWORD)OsStrtoul(ptcWord, EC_NULL, 10);
            }
        }
#endif /* INCLUDE_RAS_SPOCSUPPORT */
        else if (0 == OsStricmp(ptcWord, "-auxclk"))
        {
            pAppParms->bUseAuxClock = EC_TRUE;
            ptcWord = OsStrtok(NULL, " ");
            if ((ptcWord == EC_NULL) || (OsStrncmp(ptcWord, "-", 1) == 0))
            {
                dwRetVal = EC_E_INVALIDPARM;
                goto Exit;
            }
            pAppParms->dwBusCycleTimeUsec = OsStrtol(ptcWord, EC_NULL, 0);
            if (pAppParms->dwBusCycleTimeUsec < 10)
            {
                pAppParms->dwBusCycleTimeUsec = 10;
            }
        }
        else if (0 == OsStricmp(ptcWord, "-b"))
        {
            if (!pAppParms->bUseAuxClock)
            {
                ptcWord = OsStrtok(NULL, " ");
                if ((ptcWord == EC_NULL) || (OsStrncmp(ptcWord, "-", 1) == 0))
                {
                    dwRetVal = EC_E_INVALIDPARM;
                    goto Exit;
                }
                pAppParms->dwBusCycleTimeUsec = OsStrtol(ptcWord, EC_NULL, 0);
            }
        }        
        else if (0 == OsStricmp(ptcWord, "-ctloff"))
        {
            pAppParms->bDcmControlLoopDisabled = EC_TRUE;
        }
        else if (0 == OsStricmp(ptcWord, "-dcmmode"))
        {
            /* Extract the config file name if it was not set within quotation marks */
            ptcWord = OsStrtok(NULL, " ");

            if      (0 == OsStricmp(ptcWord, "off"))                pAppParms->eDcmMode = eDcmMode_Off;
            else if (0 == OsStricmp(ptcWord, "busshift"))           pAppParms->eDcmMode = eDcmMode_BusShift;
            else if (0 == OsStricmp(ptcWord, "mastershift"))        pAppParms->eDcmMode = eDcmMode_MasterShift;
            else if (0 == OsStricmp(ptcWord, "masterrefclock"))     pAppParms->eDcmMode = eDcmMode_MasterRefClock;
            else if (0 == OsStricmp(ptcWord, "linklayerrefclock"))  pAppParms->eDcmMode = eDcmMode_LinkLayerRefClock;
#if (defined INCLUDE_DCX)
            else if (0 == OsStricmp(ptcWord, "dcx"))                pAppParms->eDcmMode = eDcmMode_Dcx;
#endif
            else
            {
                dwRetVal = EC_E_INVALIDPARM;
                goto Exit;
            }
            pAppParms->bDcmConfigure = EC_TRUE;
        }
        else if (0 == OsStricmp(ptcWord, "-f"))
        {
            /* search config file name */
            for (nPtcWordIdx = 3; ptcWord[nPtcWordIdx] != '\0'; nPtcWordIdx++)
            {
                if ((ptcWord[nPtcWordIdx] == '\"') || (ptcWord[nPtcWordIdx] != ' '))
                {
                    break;
                }
            }
            /* extract config file name */
            if (ptcWord[nPtcWordIdx] == '\"')
            {
                if (nPtcWordIdx > 3)
                {
                    /* more than 1 blank before -f. Correct strtok position. */
                    OsStrtok(EC_NULL, "\"");
                }
                ptcWord = OsStrtok(EC_NULL, "\"");
            }
            else
            {
                ptcWord = OsStrtok(NULL, " ");
            }
            if ((ptcWord == EC_NULL) || (OsStrncmp(ptcWord, "-", 1) == 0))
            {
                dwRetVal = EC_E_INVALIDPARM;
                goto Exit;
            }
            OsSnprintf(pAppParms->szENIFilename, sizeof(pAppParms->szENIFilename) - 1, "%s", ptcWord);
            pAppParms->eCnfType = eCnfType_Filename;
            pAppParms->pbyCnfData = (EC_T_BYTE*)&pAppParms->szENIFilename;
            pAppParms->dwCnfDataLen = (EC_T_DWORD)OsStrlen(pAppParms->szENIFilename);
        }
        else if (0 == OsStricmp(ptcWord, "-flash"))
        {
            ptcWord = OsStrtok(NULL, " ");
            if ((ptcWord == EC_NULL) || (OsStrncmp(ptcWord, "-", 1) == 0))
            {
                dwRetVal = EC_E_INVALIDPARM;
                goto Exit;
            }
            pAppParms->bFlash = EC_TRUE;
            pAppParms->wFlashSlaveAddr = EC_LOWORD(OsStrtol(ptcWord, EC_NULL, 10));
        }
        else if (0 == OsStricmp(ptcWord, "-id"))
        {
            ptcWord = OsStrtok(EC_NULL, " ");
            if ((ptcWord == EC_NULL) || (OsStrncmp(ptcWord, "-", 1) == 0))
            {
                dwRetVal = EC_E_INVALIDPARM;
                goto Exit;
            }
            pAppParms->dwMasterInstanceId = OsStrtol(ptcWord, EC_NULL, 0);
        }
        else if (0 == OsStricmp(ptcWord, "-lic"))
        {
            /* search license string */
            for (nPtcWordIdx = 4; ptcWord[nPtcWordIdx] != '\0'; nPtcWordIdx++)
            {
                if (ptcWord[nPtcWordIdx] != ' ')
                {
                    break;
                }
            }
            /* extract license string */
            ptcWord = OsStrtok(NULL, " ");
            if ((ptcWord == EC_NULL) || (OsStrncmp(ptcWord, "-", 1) == 0))
            {
                dwRetVal = EC_E_INVALIDPARM;
                goto Exit;
            }
            OsSnprintf(pAppParms->szLicenseKey, sizeof(pAppParms->szLicenseKey) - 1, "%s", ptcWord);
        }
        else if (0 == OsStricmp(ptcWord, "-log"))
        {
            /* search log file name */
            for (nPtcWordIdx = 4; ptcWord[nPtcWordIdx] != '\0'; nPtcWordIdx++)
            {
                if ((ptcWord[nPtcWordIdx] == '\"') || (ptcWord[nPtcWordIdx] != ' '))
                {
                    break;
                }
            }
            /* extract log file name */
            if (ptcWord[nPtcWordIdx] == '\"')
            {
                if (nPtcWordIdx > 3)
                {
                    /* more than 1 blank before -f. Correct strtok position. */
                    OsStrtok(EC_NULL, "\"");
                }
                ptcWord = OsStrtok(EC_NULL, "\"");
            }
            else
            {
                ptcWord = OsStrtok(NULL, " ");
            }
            if ((ptcWord == EC_NULL) || (OsStrncmp(ptcWord, "-", 1) == 0))
            {
                dwRetVal = EC_E_INVALIDPARM;
                goto Exit;
            }
            OsSnprintf(pAppParms->szLogFileprefix, sizeof(pAppParms->szLogFileprefix) - 1, "%s", ptcWord);
        }
        else if (0 == OsStricmp(ptcWord, "-mbxsrv"))
        {
            ptcWord = OsStrtok(EC_NULL, " ");
            if ((ptcWord == NULL) || (0 == OsStrncmp(ptcWord, "-", 1)))
            {
                pAppParms->wMbxGatewayServerPort = EC_MBX_GATEWAY_DEFAULT_PORT;

                /* optional sub parameter not found, use the current word for the next parameter */
                bGetNextWord = EC_FALSE;
            }
            else
            {
                pAppParms->wMbxGatewayServerPort = (EC_T_WORD)OsStrtol(ptcWord, EC_NULL, 10);
            }
        }
        else if (0 == OsStricmp(ptcWord, "-notifyapp"))
        {
            ptcWord = OsStrtok(EC_NULL, " ");
            if ((ptcWord == EC_NULL) || (OsStrncmp(ptcWord, "-", 1) == 0))
            {
                dwRetVal = EC_E_INVALIDPARM;
                goto Exit;
            }
            pAppParms->dwNotifyCode = OsStrtoul(ptcWord, EC_NULL, 0);

            ptcWord = OsStrtok(EC_NULL, " ");
            if ((ptcWord == EC_NULL) || (OsStrncmp(ptcWord, "-", 1) == 0))
            {
                dwRetVal = EC_E_INVALIDPARM;
                goto Exit;
            }
            pAppParms->NotifyParms.dwInBufSize = (EC_T_BOOL)OsStrtol(ptcWord, EC_NULL, 0);

            if (pAppParms->NotifyParms.dwInBufSize > 0)
            {
                pAppParms->NotifyParms.pbyInBuf = (EC_T_BYTE*)OsMalloc(pAppParms->NotifyParms.dwInBufSize);
                if (EC_NULL == pAppParms->NotifyParms.pbyInBuf)
                {
                    dwRetVal = EC_E_NOMEMORY;
                    goto Exit;
                }
            }

            /* InBuf HexDump */
            ptcWord = OsStrtok(EC_NULL, " ");
            if ((ptcWord == EC_NULL) || (OsStrncmp(ptcWord, "-", 1) == 0))
            {
                dwRetVal = EC_E_INVALIDPARM;
                goto Exit;
            }
            {
                EC_T_CHAR* szHexDump = ptcWord;

                for (EC_T_DWORD dwInBufIx = 0; dwInBufIx < pAppParms->NotifyParms.dwInBufSize; dwInBufIx++)
                {
                    EC_T_CHAR szHexByte[3];
                    szHexByte[0] = *szHexDump++;
                    szHexByte[1] = *szHexDump++;
                    szHexByte[2] = 0;
                    pAppParms->NotifyParms.pbyInBuf[dwInBufIx] = (EC_T_BYTE)OsStrtol(szHexByte, EC_NULL, 16);
                }
            }

            ptcWord = OsStrtok(EC_NULL, " ");
            if ((ptcWord == EC_NULL) || (OsStrncmp(ptcWord, "-", 1) == 0))
            {
                dwRetVal = EC_E_INVALIDPARM;
                goto Exit;
            }
            pAppParms->NotifyParms.dwOutBufSize = (EC_T_BOOL)OsStrtol(ptcWord, EC_NULL, 0);
            if (pAppParms->NotifyParms.dwOutBufSize > 0)
            {
                pAppParms->NotifyParms.pbyOutBuf = (EC_T_BYTE*)OsMalloc(pAppParms->NotifyParms.dwOutBufSize);
            }

            pAppParms->NotifyParms.pdwNumOutData = (EC_T_DWORD*)OsMalloc(sizeof(EC_T_DWORD));
        }
        else if (0 == OsStricmp(ptcWord, "-perf"))
        {
            pAppParms->bPerfMeasEnabled = EC_TRUE;
        }
#if (defined INCLUDE_PCAP_RECORDER)
        else if (0 == OsStricmp(ptcWord, "-rec"))
        {
            pAppParms->bPcapRecorder = EC_TRUE;
        }
#endif
        else if (0 == OsStricmp(ptcWord, "-rem"))
        {
            EC_T_CHAR*  ptcTmp = EC_NULL;
            EC_T_INT    nCnt = 0;

            /* get next word */
            ptcWord = OsStrtok(EC_NULL, ".");

            if ((ptcWord == EC_NULL) || (OsStrncmp(ptcWord, "-", 1) == 0))
            {
                dwRetVal = EC_E_INVALIDPARM;
                goto Exit;
            }
            /* get IP address */
            ptcTmp = ptcWord;
            for (nCnt = 0; nCnt < 5; nCnt++)
            {
                if (ptcTmp == EC_NULL)
                {
                    dwRetVal = EC_E_INVALIDPARM;
                    goto Exit;
                }
                if (nCnt < 4)
                {
                    pAppParms->abyRasServerIpAddress[nCnt] = (EC_T_BYTE)OsStrtol(ptcTmp, EC_NULL, 0);
                }
                else
                {
                    pAppParms->wRasServerPort = (EC_T_WORD)OsStrtol(ptcTmp, EC_NULL, 0);
                }
                if (nCnt < 2)
                {
                    ptcTmp = OsStrtok(EC_NULL, ".");
                }
                else if (nCnt < 3)
                {
                    ptcTmp = OsStrtok(EC_NULL, ":");
                }
                else if (nCnt < 4)
                {
                    ptcTmp = OsStrtok(EC_NULL, " ");
                }
            }
        }
        else if (0 == OsStricmp(ptcWord, "-rmod"))
        {
            pAppParms->bReadMasterOd = EC_TRUE;
        }
        else if (0 == OsStricmp(ptcWord, "-setpdbits"))
        {
            EC_T_CHAR* ptcTmp = OsStrtok(EC_NULL, " ");
            pAppParms->SetProcessDataBits.dwOffset = (EC_T_DWORD)OsStrtol(ptcTmp, EC_NULL, 0); ptcTmp = OsStrtok(EC_NULL, " ");
            pAppParms->SetProcessDataBits.dwSize = (EC_T_DWORD)OsStrtol(ptcTmp, EC_NULL, 0); ptcTmp = OsStrtok(EC_NULL, " ");
            pAppParms->SetProcessDataBits.dwValue = (EC_T_DWORD)OsStrtol(ptcTmp, EC_NULL, 0); ptcTmp = OsStrtok(EC_NULL, " ");

            /* optional sub parameter duration */
            if ((ptcTmp == NULL) || (OsStrncmp(ptcTmp, "-", 1) == 0))
            {
                pAppParms->SetProcessDataBits.dwDuration = 0;
                bGetNextWord = EC_FALSE;
                ptcWord = ptcTmp;
            }
            else
            {
                pAppParms->SetProcessDataBits.dwDuration = (EC_T_WORD)OsStrtol(ptcTmp, EC_NULL, 10);
            }
        }
#if (defined ATEMRAS_SERVER)
        else if (0 == OsStricmp(ptcWord, "-sp"))
        {
            ptcWord = OsStrtok(NULL, " ");
            if ((ptcWord == NULL) || (OsStrncmp(ptcWord, "-", 1) == 0))
            {
                pAppParms->wRasServerPort = ATEMRAS_DEFAULT_PORT;

                /* optional sub parameter not found, use the current word for the next parameter */
                bGetNextWord = EC_FALSE;
            }
            else
            {
                pAppParms->wRasServerPort = (EC_T_WORD)OsStrtol(ptcWord, EC_NULL, 10);
            }
        }
#endif
        else if (0 == OsStricmp(ptcWord, "-standby"))
        {
            pAppParms->bMasterRedPermanentStandby = EC_TRUE;
        }        
        else if (0 == OsStricmp(ptcWord, "-t"))
        {
            ptcWord = OsStrtok(NULL, " ");
            if ((ptcWord == EC_NULL) || (OsStrncmp(ptcWord, "-", 1) == 0))
            {
                dwRetVal = EC_E_INVALIDPARM;
                goto Exit;
            }
            pAppParms->dwDemoDuration = OsStrtol(ptcWord, EC_NULL, 0);
        }        
        else if (0 == OsStricmp(ptcWord, "-v"))
        {
            ptcWord = OsStrtok(NULL, " ");
            if ((ptcWord == EC_NULL) || (OsStrncmp(ptcWord, "-", 1) == 0))
            {
                dwRetVal = EC_E_INVALIDPARM;
                goto Exit;
            }
            nVerbose = OsStrtol(ptcWord, EC_NULL, 10);
        }
#if (defined INCLUDE_DAQ_SUPPORT)
        else if (0 == OsStricmp(ptcWord, "-daqrec"))
        {
            ptcWord = OsStrtok(NULL, " ");
            if ((ptcWord == EC_NULL) || (OsStrncmp(ptcWord, "-", 1) == 0))
            {
                dwRetVal = EC_E_INVALIDPARM;
                goto Exit;
            }
            pAppParms->bDaqRecorder = EC_TRUE;
            OsSnprintf(pAppParms->szDaqRecorder, sizeof(pAppParms->szDaqRecorder) - 1, "%s", ptcWord);
        }
#endif
#if (defined EC_SIMULATOR_DS402)
        else if (0 == OsStricmp(ptcWord, "-ds402"))
        {
            ptcWord = OsStrtok(NULL, " ");
            if ((ptcWord == EC_NULL) || (OsStrncmp(ptcWord, "-", 1) == 0))
            {
                dwRetVal = EC_E_INVALIDPARM;
                goto Exit;
            }

            pAppParms->dwDS402NumSlaves = 0;
            for (;;)
            {
                EC_T_CHAR* ptcTempWord = ptcWord;
                EC_T_WORD wAddr = (EC_T_WORD)strtol(ptcWord, &ptcWord, 10);

                if (ptcWord == ptcTempWord)
                {
                    break;
                }
                
                if (pAppParms->dwDS402NumSlaves == DEMO_MAX_NUM_OF_AXIS)
                {
                    dwRetVal = EC_E_INVALIDPARM;
                    goto Exit;
                }

                pAppParms->awDS402SlaveAddr[pAppParms->dwDS402NumSlaves++] = wAddr;

                if (*ptcWord == '\0')
                {
                    break;
                }
                ptcWord++;
            }
        }
#endif
#if (!defined ATEMRAS_CLIENT)
        else
        {
            EC_T_DWORD dwRes = EC_E_NOERROR;

            dwRes = CreateLinkParmsFromCmdLine(&ptcWord, (EC_T_CHAR**)&szCommandLineTmp, &tcStorage, &bGetNextWord, 
                &pAppParms->apLinkParms[dwNumLinkLayer], &pAppParms->TtsParms);
            if (EC_E_NOERROR != dwRes)
            {
                dwRetVal = EC_E_INVALIDPARM;
                goto Exit;
            }
            dwNumLinkLayer++;
        }
#endif /* !ATEMRAS_CLIENT */
        /* get next word */
        if (bGetNextWord)
        {
            ptcWord = OsStrtok(NULL, " ");
        }
        bGetNextWord = EC_TRUE;
    }

    pAppParms->nVerbose = nVerbose;
    switch (nVerbose)
    {
    case 0:
        pAppParms->dwAppLogLevel   = EC_LOG_LEVEL_SILENT;
        pAppParms->dwMasterLogLevel = EC_LOG_LEVEL_SILENT;
        break;
    case 1:
        pAppParms->dwAppLogLevel   = EC_LOG_LEVEL_INFO;
        pAppParms->dwMasterLogLevel = EC_LOG_LEVEL_ERROR;
        break;
    case 2:
        pAppParms->dwAppLogLevel   = EC_LOG_LEVEL_INFO;
        pAppParms->dwMasterLogLevel = EC_LOG_LEVEL_WARNING;
        pAppParms->bPerfMeasShowCyclic = pAppParms->bPerfMeasEnabled;
        break;
    case 3:
        pAppParms->dwAppLogLevel   = EC_LOG_LEVEL_VERBOSE;
        pAppParms->dwMasterLogLevel = EC_LOG_LEVEL_WARNING;     
        pAppParms->bPerfMeasShowCyclic = pAppParms->bPerfMeasEnabled;
        pAppParms->bDcmLogEnabled   = EC_TRUE;
        break;
    case 4:
        pAppParms->dwAppLogLevel   = EC_LOG_LEVEL_VERBOSE;
        pAppParms->dwMasterLogLevel = EC_LOG_LEVEL_INFO;
        pAppParms->bPerfMeasShowCyclic = pAppParms->bPerfMeasEnabled;
        pAppParms->bDcmLogEnabled   = EC_TRUE;
        break;
    case 5:  
        pAppParms->dwAppLogLevel   = EC_LOG_LEVEL_VERBOSE;      
        pAppParms->dwMasterLogLevel = EC_LOG_LEVEL_VERBOSE;
        pAppParms->bPerfMeasShowCyclic = pAppParms->bPerfMeasEnabled;
        pAppParms->bDcmLogEnabled   = EC_TRUE;
        break;
    default: 
        pAppParms->dwAppLogLevel   = EC_LOG_LEVEL_VERBOSE_CYC;
        pAppParms->dwMasterLogLevel = EC_LOG_LEVEL_VERBOSE_CYC;
        pAppParms->bPerfMeasShowCyclic = EC_TRUE;
        pAppParms->bDcmLogEnabled   = EC_TRUE;
        break;
    }
    /* no errors */
    dwRetVal = EC_E_NOERROR;

Exit:
    if (EC_E_NOERROR != dwRetVal)
    {
        EcLogMsg(EC_LOG_LEVEL_ERROR, (pEcLogContext, EC_LOG_LEVEL_ERROR, "Full command line: %s\n", szCommandLine));
        EcLogMsg(EC_LOG_LEVEL_ERROR, (pEcLogContext, EC_LOG_LEVEL_ERROR, "Failed to parse command line parameters\n"));
    }
    return dwRetVal;
}

EC_T_VOID ShowSyntaxCommon(T_EC_DEMO_APP_CONTEXT* pAppContext)
{
    EcLogMsg(EC_LOG_LEVEL_ERROR, (pEcLogContext, EC_LOG_LEVEL_ERROR, "   -f                Use given ENI file\n"));
    EcLogMsg(EC_LOG_LEVEL_ERROR, (pEcLogContext, EC_LOG_LEVEL_ERROR, "     FileName        file name .xml\n"));
    EcLogMsg(EC_LOG_LEVEL_ERROR, (pEcLogContext, EC_LOG_LEVEL_ERROR, "   -t                Demo duration\n"));
    EcLogMsg(EC_LOG_LEVEL_ERROR, (pEcLogContext, EC_LOG_LEVEL_ERROR, "     time            Time in msec, 0 = forever (default = 120000)\n"));
    EcLogMsg(EC_LOG_LEVEL_ERROR, (pEcLogContext, EC_LOG_LEVEL_ERROR, "   -b                Bus cycle time\n"));
    EcLogMsg(EC_LOG_LEVEL_ERROR, (pEcLogContext, EC_LOG_LEVEL_ERROR, "     cycle time      Cycle time in usec\n"));
    EcLogMsg(EC_LOG_LEVEL_ERROR, (pEcLogContext, EC_LOG_LEVEL_ERROR, "   -a                CPU affinity\n"));
    EcLogMsg(EC_LOG_LEVEL_ERROR, (pEcLogContext, EC_LOG_LEVEL_ERROR, "     affinity        0 = first CPU, 1 = second, ...\n"));
    EcLogMsg(EC_LOG_LEVEL_ERROR, (pEcLogContext, EC_LOG_LEVEL_ERROR, "   -v                Set verbosity level\n"));
    EcLogMsg(EC_LOG_LEVEL_ERROR, (pEcLogContext, EC_LOG_LEVEL_ERROR, "     lvl             Level: 0=off, 1...n=more messages, 3(default) generate dcmlog file\n"));
    EcLogMsg(EC_LOG_LEVEL_ERROR, (pEcLogContext, EC_LOG_LEVEL_ERROR, "   -perf             Enable performance measurement (printed cyclically if verbosity level >= 2)\n"));
    EcLogMsg(EC_LOG_LEVEL_ERROR, (pEcLogContext, EC_LOG_LEVEL_ERROR, "   -log              Use given file name prefix for log files\n"));
    EcLogMsg(EC_LOG_LEVEL_ERROR, (pEcLogContext, EC_LOG_LEVEL_ERROR, "     prefix          prefix\n"));
    EcLogMsg(EC_LOG_LEVEL_ERROR, (pEcLogContext, EC_LOG_LEVEL_ERROR, "   -lic              Use License key\n"));
    EcLogMsg(EC_LOG_LEVEL_ERROR, (pEcLogContext, EC_LOG_LEVEL_ERROR, "     key             license key\n"));
#if (defined ATEMRAS_SERVER)
    EcLogMsg(EC_LOG_LEVEL_ERROR, (pEcLogContext, EC_LOG_LEVEL_ERROR, "   -sp               Start RAS server\n"));
    EcLogMsg(EC_LOG_LEVEL_ERROR, (pEcLogContext, EC_LOG_LEVEL_ERROR, "    [port            port (default = %d)]\n", ATEMRAS_DEFAULT_PORT));
#endif
#if (defined EC_SIMULATOR_DS402)
    EcLogMsg(EC_LOG_LEVEL_ERROR, (pEcLogContext, EC_LOG_LEVEL_ERROR, "   -ds402            Simulate DS402 profile for given slaves\n"));
    EcLogMsg(EC_LOG_LEVEL_ERROR, (pEcLogContext, EC_LOG_LEVEL_ERROR, "     Addr1,...,AddrN Comma separated list of station fixed adresses\n"));
#endif
}

/*-END OF SOURCE FILE--------------------------------------------------------*/
