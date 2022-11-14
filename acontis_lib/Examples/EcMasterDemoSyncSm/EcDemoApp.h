/*-----------------------------------------------------------------------------
 * EcDemoApp.h
 * Copyright                acontis technologies GmbH, Ravensburg, Germany
 * Response                 Holger Oelhaf
 * Description              Application specific settings for EC-Master demo
 *---------------------------------------------------------------------------*/

#ifndef INC_ECDEMOAPP_H
#define INC_ECDEMOAPP_H 1

/*-LOGGING-------------------------------------------------------------------*/
#ifndef pEcLogParms
#define pEcLogParms (&(pAppContext->LogParms))
#endif

/*-INCLUDES------------------------------------------------------------------*/
#include "AtEthercat.h"
#include "EcDemoPlatform.h"
#include "EcDemoParms.h"
#include "EcLogging.h"
#include "EcNotification.h"
#include "EcSdoServices.h"
#include "EcSelectLinkLayer.h"
#include "EcSlaveInfo.h"

/*-DEFINES-------------------------------------------------------------------*/
#define EC_DEMO_APP_NAME (EC_T_CHAR*)"EcMasterDemoSyncSm"

/* the RAS server is necessary to support the EC-Engineer or other remote applications */
#if (!defined ATEMRAS_SERVER) && (defined EC_SOCKET_SUPPORTED)
#define ATEMRAS_SERVER
#endif

#if (defined ATEMRAS_SERVER)
#include "AtEmRasSrv.h"
#define ATEMRAS_MAX_WATCHDOG_TIMEOUT    10000
#define ATEMRAS_CYCLE_TIME              2
#endif

/*--------------------------------------------------------------------------*/
/* Performance measurements of jobs                                         */
/* This is only available on CPUs with TSC support                          */
/*--------------------------------------------------------------------------*/
#define PERF_MEASURE_JOBS_INIT(numJobs) ecatPerfMeasInit(&pAppContext->TscMeasDesc,0,numJobs,EC_NULL);ecatPerfMeasEnable(&pAppContext->TscMeasDesc)
#define PERF_MEASURE_JOBS_DEINIT()      ecatPerfMeasDeinit(&pAppContext->TscMeasDesc)
#define PERF_MEASURE_JOBS_RESET()       ecatPerfMeasReset(&pAppContext->TscMeasDesc, 0xFFFFFFFF);
#define PERF_MEASURE_JOBS_SHOW()        ecatPerfMeasShow(&pAppContext->TscMeasDesc,0xFFFFFFFF,S_aszMeasInfo)
#define PERF_JOB_START(nJobIndex)       ecatPerfMeasStart(&pAppContext->TscMeasDesc,(EC_T_DWORD)(nJobIndex))
#define PERF_JOB_END(nJobIndex)         ecatPerfMeasEnd(&pAppContext->TscMeasDesc,(EC_T_DWORD)(nJobIndex))

/*-FUNCTION DECLARATIONS-----------------------------------------------------*/
EC_T_VOID  ShowSyntaxAppUsage(T_EC_DEMO_APP_CONTEXT* pAppContext);
EC_T_VOID  ShowSyntaxApp(T_EC_DEMO_APP_CONTEXT* pAppContext);
EC_T_VOID  ShowSyntaxLinkLayer(EC_T_VOID);
EC_T_DWORD EcMasterApp(T_EC_DEMO_APP_CONTEXT* pAppContext);

#endif /* INC_ECDEMOAPP_H */

/*-END OF SOURCE FILE--------------------------------------------------------*/
