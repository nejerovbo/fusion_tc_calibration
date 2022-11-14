/*-----------------------------------------------------------------------------
 * EcLinkDummy.h
 * Copyright                acontis technologies GmbH, Ravensburg, Germany
 * Response                 
 * Description              EtherCAT Master link layer interface
 *---------------------------------------------------------------------------*/

#ifndef INC_ECLINKDUMMY
#define INC_ECLINKDUMMY

/*-INCLUDES------------------------------------------------------------------*/
#ifndef INC_ECLINK
#include <EcLink.h>
#endif

/*-COMPILER SETTINGS---------------------------------------------------------*/
#ifdef __cplusplus
extern "C" {
#endif

/*-DEFINES-------------------------------------------------------------------*/
#define EC_LINK_PARMS_SIGNATURE_DUMMY_PATTERN (EC_T_DWORD)0x0000FFF0
#define EC_LINK_PARMS_SIGNATURE_DUMMY_VERSION (EC_T_DWORD)0x00000001
#define EC_LINK_PARMS_SIGNATURE_DUMMY (EC_T_DWORD)(EC_LINK_PARMS_SIGNATURE|EC_LINK_PARMS_SIGNATURE_DUMMY_PATTERN|EC_LINK_PARMS_SIGNATURE_DUMMY_VERSION)
#define EC_LINK_PARMS_IDENT_DUMMY   "Dummy"

#include EC_PACKED_API_INCLUDESTART
typedef struct _EC_T_LINK_PARMS_DUMMY
{
    /**
    * \brief Link Layer abstraction.
    * set linkParms.dwSignature = EC_LINK_PARMS_SIGNATURE_DUMMY
    *
    * Must be first, see casts in usage.
    */
    EC_T_LINK_PARMS linkParms;

    EC_T_DWORD      dwPhyAddr;              /* [in]  PHY address (0 .. 31) on MII bus */
    EC_T_DWORD      dwRegisterBasePhys;     /* [in] Physical base address of register block (8k) */

} EC_PACKED_API EC_T_LINK_PARMS_DUMMY;
#include EC_PACKED_INCLUDESTOP

/*-FUNCTIONS DECLARATION-----------------------------------------------------*/
ATEMLL_API EC_T_DWORD emllRegisterDummy
    (EC_T_LINK_DRV_DESC*  pLinkDrvDesc        /* [in,out] link layer driver descriptor */
    , EC_T_DWORD        dwLinkDrvDescSize);   /* [in]     size in bytes of link layer driver descriptor */

/*-COMPILER SETTINGS---------------------------------------------------------*/
#ifdef __cplusplus
}
#endif

#endif /* INC_ECLINK */

/*-END OF SOURCE FILE--------------------------------------------------------*/
