#include "global.h"
#include "main.h"
#include "dns.h"
#include "palette.h"
#include "start_menu.h"
#include "overworld.h"
#include "battle_main.h"
#include "rtc.h"
#include "constants/map_types.h"
#include "constants/rgb.h"
#include "constants/dns_constants.h" // Include your constants

/*******************************************************/
/*********    Day and Night Configuration     **********/
/********************************************************
 * You can customize the DNS by editing the following   *
 * timelapses and the filters used to change the        *
 * palette colours.                                     *
 * In addition to that, you can also configure which    *
 * palettes are affected by the system, as well as      *
 * establishing sprite palettes exceptions by its TAG.  *
 *                                                      *
 * It is highly recommended to read the following config*
 * options, to understand how the dns works.            *
 * ******************************************************/


/* End hours for each of the timelapses */
#define MIDNIGHT_END_HOUR   7       //00 - 07
#define DAWN_END_HOUR       8       //07 - 08
#define DAY_END_HOUR        19      //08 - 19
#define SUNSET_END_HOUR     20      //19 - 20
#define NIGHTFALL_END_HOUR  21      //20 - 21
#define NIGHT_END_HOUR      0       //21 - 00

/* Start and end hour of the lightning system.
 * This system is generally used for building's windows. */
#define LIGHTNING_START_HOUR    NIGHTFALL_END_HOUR
#define LIGHTNING_END_HOUR      MIDNIGHT_END_HOUR

/* This array contains the colours used for the windows or          *
 * other tiles that have to be illuminated at night.                *
 * You can add or remove light slots as you wish, each entry       *
 * requires the paletteNum and the colourNum of each colour slot,   *
 * as well as the RGB 15 bit colour that's gonna be used as         *
 * "light colour".                                                  */
const struct LightingColour gLightingColours[] =
{
    {
        .paletteNum = 0,
        .colourNum = 1,
        .lightColour = RGB2(30, 30, 5),
    },
    {
        .paletteNum = 0,
        .colourNum = 2,
        .lightColour = RGB2(26, 25, 4),
    },
    {
        .paletteNum = 0,
        .colourNum = 3,
        .lightColour = RGB2(22, 21, 3),
    },
    {
        .paletteNum = 1,
        .colourNum = 1,
        .lightColour = RGB2(30, 30, 5),
    },
    {
        .paletteNum = 1,
        .colourNum = 2,
        .lightColour = RGB2(26, 25, 4),
    },
    {
        .paletteNum = 6,
        .colourNum = 1,
        .lightColour = RGB2(30, 30, 5),
    },
    {
        .paletteNum = 6,
        .colourNum = 2,
        .lightColour = RGB2(26, 25, 4),
    },
    {
        .paletteNum = 6,
        .colourNum = 3,
        .lightColour = RGB2(22, 21, 3),
    },
};

/* Maptypes that are not affected by DNS */
const u8 gDnsMapExceptions[] =
{
    MAP_TYPE_NONE,
    MAP_TYPE_INDOOR,
    MAP_TYPE_UNDERGROUND,
    MAP_TYPE_SECRET_BASE,
};

/* Configure each palette slot to be affected or not by DNS *
 * while you are in the overworld.                          */
const struct DnsPalExceptions gOWPalExceptions = 
{
    .pal = {
        DNS_PAL_ACTIVE,     //0
        DNS_PAL_ACTIVE,     //1
        DNS_PAL_ACTIVE,     //2
        DNS_PAL_ACTIVE,     //3
        DNS_PAL_ACTIVE,     //4
        DNS_PAL_ACTIVE,     //5
        DNS_PAL_ACTIVE,     //6
        DNS_PAL_ACTIVE,     //7
        DNS_PAL_ACTIVE,     //8
        DNS_PAL_ACTIVE,     //9
        DNS_PAL_ACTIVE,     //10
        DNS_PAL_ACTIVE,     //11
        DNS_PAL_ACTIVE,     //12
        DNS_PAL_EXCEPTION,  //13
        DNS_PAL_EXCEPTION,  //14
        DNS_PAL_EXCEPTION,  //15
        DNS_PAL_ACTIVE,     //16
        DNS_PAL_ACTIVE,     //17
        DNS_PAL_ACTIVE,     //18
        DNS_PAL_ACTIVE,     //19
        DNS_PAL_ACTIVE,     //20
        DNS_PAL_ACTIVE,     //21
        DNS_PAL_ACTIVE,     //22
        DNS_PAL_ACTIVE,     //23
        DNS_PAL_ACTIVE,     //24
        DNS_PAL_ACTIVE,     //25
        DNS_PAL_ACTIVE,     //26
        DNS_PAL_ACTIVE,     //27
        DNS_PAL_ACTIVE,     //28
        DNS_PAL_ACTIVE,     //29
        DNS_PAL_ACTIVE,     //30
        DNS_PAL_ACTIVE,     //31
    }
};

/* Configure each palette slot to be affected or not by DNS *
 * while in combat.                                         */
const struct DnsPalExceptions gCombatPalExceptions =  
{
    .pal = {
        DNS_PAL_EXCEPTION,  //0
        DNS_PAL_EXCEPTION,  //1
        DNS_PAL_ACTIVE,     //2
        DNS_PAL_ACTIVE,     //3
        DNS_PAL_ACTIVE,     //4
        DNS_PAL_EXCEPTION,  //5
        DNS_PAL_ACTIVE,     //6
        DNS_PAL_ACTIVE,     //7
        DNS_PAL_ACTIVE,     //8
        DNS_PAL_ACTIVE,     //9
        DNS_PAL_ACTIVE,     //10
        DNS_PAL_ACTIVE,     //11
        DNS_PAL_ACTIVE,     //12
        DNS_PAL_ACTIVE,     //13
        DNS_PAL_ACTIVE,     //14
        DNS_PAL_ACTIVE,     //15
        DNS_PAL_EXCEPTION,  //16
        DNS_PAL_EXCEPTION,  //17
        DNS_PAL_EXCEPTION,  //18
        DNS_PAL_EXCEPTION,  //19
        DNS_PAL_EXCEPTION,  //20
        DNS_PAL_EXCEPTION,  //21
        DNS_PAL_EXCEPTION,  //22
        DNS_PAL_EXCEPTION,  //23
        DNS_PAL_EXCEPTION,  //24
        DNS_PAL_EXCEPTION,  //25
        DNS_PAL_EXCEPTION,  //26
        DNS_PAL_EXCEPTION,  //27
        DNS_PAL_EXCEPTION,  //28
        DNS_PAL_EXCEPTION,  //29
        DNS_PAL_EXCEPTION,  //30
        DNS_PAL_EXCEPTION,  //31
    }
};

/*******************************************************/
/*************    DNS Colour Filters     ***************/
/*******************************************************/
/* DNS filters are actual 15bit RGB colours.            *
 * This colours R - G - B channels are subtracted from *
 * the original colour in the palette buffer during the *
 * transfer from the buffer to the palette RAM.         *
 *                                                      *
 *  [BUFFER] -> (Value - Filter) -> [PAL_RAM]           *
 *                                                      *
 * This means that you shouldn't use too high values for *
 * RGB channels in the filters. Otherwise, the channels  *
 * will easily reach 0, giving you plain colours.       *
 * I suggest to not use channels with a value above 16. *
 *                                                      *
 * Feel free to experiment with your own filters.       *
 */

/* Filters used at midnight.                    *
 * From 00:00 to 01:00 filters are cycled every *
 * 8 minutes.                                   *
 * From 01:00 to 07:00 the last filter is used. */
const u16 gMidnightFilters[] =
{
    RGB2(14, 14, 6),    
    RGB2(14, 14, 7),    
    RGB2(14, 14, 8),    
    RGB2(15, 15, 8),    
    RGB2(15, 15, 9),    
    RGB2(15, 15, 9),    
    RGB2(16, 16, 9),   
    RGB2(16, 16, 10),
};

/* Filters used at dawn. (30 filters).          *
 * From 07:00 to 08:00 filters are cycled every *
 * 2 minutes.                                   */
const u16 gDawnFilters[] =
{
    RGB2(15, 15, 10),
    RGB2(15, 15, 10),   
    RGB2(14, 14, 10),   
    RGB2(13, 13, 10),   
    RGB2(12, 12, 10),   
    RGB2(11, 11, 10),   
    RGB2(10, 10, 10),   
    RGB2(9, 9, 10),     
    RGB2(8, 8, 10),     
    RGB2(8, 8, 11),     
    RGB2(7, 7, 11),     
    RGB2(6, 6, 11),     
    RGB2(5, 5, 11),     
    RGB2(4, 4, 11),     
    RGB2(3, 3, 11),     
    RGB2(2, 2, 11),     
    RGB2(1, 1, 11),     
    RGB2(0, 0, 11),     
    RGB2(0, 0, 10),     
    RGB2(0, 0, 9),      
    RGB2(0, 0, 8),      
    RGB2(0, 0, 7),      
    RGB2(0, 0, 6),      
    RGB2(0, 0, 5),      
    RGB2(0, 0, 4),      
    RGB2(0, 0, 3),      
    RGB2(0, 0, 2),      
    RGB2(0, 0, 1),      
    RGB2(0, 0, 0),      
    RGB2(0, 0, 0),      
};

/* Filters used at day. (no filter actually lul)*/
const u16 gDayFilter = RGB2(0, 0, 0);   

/* Filters used at sunset. (30 filters).        *
 * From 19:00 to 20:00 filters are cycled every *
 * 2 minutes.                                   */
const u16 gSunsetFilters[] = 
{
    RGB2(0, 0, 1),      
    RGB2(0, 1, 1),      
    RGB2(0, 1, 2),      
    RGB2(0, 1, 3),      
    RGB2(0, 2, 3),      
    RGB2(0, 2, 4),      
    RGB2(0, 2, 5),      
    RGB2(0, 3, 5),      
    RGB2(0, 3, 6),      
    RGB2(0, 3, 7),      
    RGB2(0, 4, 7),      
    RGB2(0, 4, 8),      
    RGB2(0, 4, 9),      
    RGB2(0, 5, 9),      
    RGB2(0, 5, 10),     
    RGB2(0, 5, 11),     
    RGB2(0, 6, 11),     
    RGB2(0, 6, 12),     
    RGB2(0, 6, 13),     
    RGB2(0, 7, 13),     
    RGB2(0, 7, 14),     
    RGB2(0, 7, 14),     
    RGB2(0, 8, 14),     
    RGB2(0, 9, 14),     
    RGB2(0, 10, 14),    
    RGB2(0, 11, 14),    
    RGB2(0, 12, 14),    
    RGB2(0, 13, 14),    
    RGB2(0, 14, 14),    
    RGB2(0, 14, 14),    
};

/* Filters used at nightfall. (30 filters).     *
 * From 20:00 to 21:00 filters are cycled every *
 * 2 minutes.                                   */
const u16 gNightfallFilters[] = 
{
    RGB2(0, 14, 14),    
    RGB2(0, 14, 14),    
    RGB2(0, 14, 13),    
    RGB2(0, 14, 12),    
    RGB2(0, 14, 11),    
    RGB2(0, 14, 10),    
    RGB2(1, 14, 10),    
    RGB2(1, 14, 9),     
    RGB2(0, 14, 8),     
    RGB2(1, 14, 7),     
    RGB2(1, 14, 6),     
    RGB2(2, 14, 6),     
    RGB2(2, 14, 5),     
    RGB2(2, 14, 4),     
    RGB2(2, 14, 3),     
    RGB2(2, 14, 2),     
    RGB2(2, 14, 2),     
    RGB2(3, 14, 3),     
    RGB2(4, 14, 4),     
    RGB2(5, 14, 5),     
    RGB2(6, 14, 6),     
    RGB2(7, 14, 6),     
    RGB2(8, 14, 6),     
    RGB2(9, 14, 6),     
    RGB2(10, 14, 6),    
    RGB2(11, 14, 6),    
    RGB2(12, 14, 6),    
    RGB2(13, 14, 6),    
    RGB2(14, 14, 6),    
    RGB2(14, 14, 6),    
};

/* Filter used at night. From 21:00 to 24:00 */
const u16 gNightFilter = RGB2(14, 14, 6);   

/*************   SpritePalette Dns exceptions by TAG   **************
 * If you are using any dynamic sprite palette allocation system,   *
 * you will most likely want to use this system to avoid certain    *
 * palette tags to be "banned" from dns, as the palettes may get    *
 * loaded in different slots each time.                             */
const u16 gPaletteTagExceptions[] =
{
    0xD6FF, //TAG_HEALTHBOX_PAL
    0xD704, //TAG_HEALTHBAR_PAL
    0xD710, //TAG_STATUS_SUMMARY_BAR_PAL
    0xD712, //TAG_STATUS_SUMMARY_BALLS_PAL
};

/***********************************************
 * --------- DNS CONFIGURATION END ----------- *
 ******************************************* */

// Function Declarations
static u16 DnsApplyFilterToColour(u16 colour, u16 filter);
static u16 DnsApplyProportionalFilterToColour(u16 colour, u16 filter);
static void DoDnsLightning();
static u16 GetDNSFilter();  // Define this function based on your previous requirements
static bool8 IsMapDNSException();
static bool8 IsSpritePaletteTagDnsException(u8 palNum);
static bool8 IsOverworld();
static bool8 IsCombat();
static bool8 IsLightActive();

// DNS palette buffer in EWRAM
ALIGNED(4) EWRAM_DATA static u16 sDnsPaletteDmaBuffer[512] = {0 };


// Called from TransferPlttBuffer
void DnsTransferPlttBuffer(void *src, void *dest)
{
    if ((IsOverworld() || IsCombat()) && !IsMapDNSException()) 
    {
        DmaCopy16(3, sDnsPaletteDmaBuffer, dest, PLTT_SIZE);
    }
    else
    {
        DmaCopy16(3, src, dest, PLTT_SIZE);
    }
}

/* Applies filter to palette colours, stores new palettes in EWRAM buffer.   *
 * It must be called from CB2 if the DNS wants to be used (similar to        *
 * TransferPlttBuffer) in VBlank callbacks                                  */
void DnsApplyFilters()
{
    u8 palNum, colNum;
    u16 rgbFilter;
    struct DnsPalExceptions palExceptionFlags;

    rgbFilter = GetDNSFilter();

    palExceptionFlags = gMain.inBattle ? gCombatPalExceptions : gOWPalExceptions; // Init pal exception slots

    for (palNum = 0; palNum < 32; palNum++)
    {
        if (palExceptionFlags.pal[palNum] && (palNum < 15 || !IsSpritePaletteTagDnsException(palNum - 16)))
        {
            for (colNum = 0; colNum < 16; colNum++) // Transfer filtered palette to buffer
            {
                sDnsPaletteDmaBuffer[palNum * 16 + colNum] = DnsApplyProportionalFilterToColour(gPlttBufferFaded[palNum * 16 + colNum], rgbFilter);
            }
        }
        else
        {
            for (colNum = 0; colNum < 16; colNum++) // Transfers palette to buffer without filtering
            {
                sDnsPaletteDmaBuffer[palNum * 16 + colNum] = gPlttBufferFaded[palNum * 16 + colNum];      
            }      
        }
    }

    if (!IsMapDNSException() && IsLightActive() && !gMain.inBattle)
    {
        DoDnsLightning();
    }
}

// Applies filter to a colour
static u16 DnsApplyFilterToColour(u16 colour, u16 filter)
{
    u16 red, green, blue;

    red = (colour & 0x1F) - (filter & 0x1F);
    green = ((colour & 0x3E0) - (filter & 0x3E0)) >> 5;
    blue = ((colour & 0x7C00) - (filter & 0x7C00)) >> 10;

    return RGB2(red <= 31 ? red : 0, green <= 31 ? green : 0, blue <= 31 ? blue : 0);
}

// Alternative way to apply filter
static u16 DnsApplyProportionalFilterToColour(u16 colour, u16 filter)
{
    u32 red, green, blue;

    red = (colour & 0x1F) * (0x1F - (filter & 0x1F)) >> 5;
    green = ((colour & 0x3E0) >> 5) * ((0x3E0 - (filter & 0x3E0)) >> 5) >> 5;
    blue = ((colour & 0x7C00) >> 10) * ((0x7C00 - (filter & 0x7C00)) >> 10) >> 5;

    return RGB2(red <= 31 ? red : 0, green <= 31 ? green : 0, blue <= 31 ? blue : 0);  
}

static u16 GetDNSFilter()
{
    u8 hour = gLocalTime.hours;    
    u8 minutes = gLocalTime.minutes;   

    switch(GetDnsTimeLapse(hour))
    {
        case DNS_TIME_MIDNIGHT:
            if (hour < 1)
                return gMidnightFilters[minutes >> 3];            
            else
                return gMidnightFilters[7];

        case DNS_TIME_DAWN:
            return gDawnFilters[minutes >> 1];

        case DNS_TIME_DAY:
            return gDayFilter;

        case DNS_TIME_SUNSET: 
            return gSunsetFilters[minutes >> 1];

        case DNS_TIME_NIGHTFALL:
            return gNightfallFilters[minutes >> 1];

        case DNS_TIME_NIGHT:
            return gNightFilter;
    }

    return 0;
}

// Function to get the current time lapse
u8 GetDnsTimeLapse(u8 hour) {
    if (hour < MIDNIGHT_END_HOUR)
        return DNS_TIME_MIDNIGHT;
    else if (hour < DAWN_END_HOUR)
        return DNS_TIME_DAWN;
    else if (hour < DAY_END_HOUR)
        return DNS_TIME_DAY;
    else if (hour < SUNSET_END_HOUR)
        return DNS_TIME_SUNSET;
    else if (hour < NIGHTFALL_END_HOUR)
        return DNS_TIME_NIGHTFALL;
    else
        return DNS_TIME_NIGHT;  // This covers all cases now
}

// Function for managing lightning effects
static void DoDnsLightning()
{
    u8 i;

    for (i = 0; i < sizeof(gLightingColours)/sizeof(gLightingColours[0]); i++)
    {
        u16 colourSlot = gLightingColours[i].paletteNum * 16 + gLightingColours[i].colourNum;
        
        if (gPaletteFade.active || gPlttBufferUnfaded[colourSlot] != 0x0000)
        {
            sDnsPaletteDmaBuffer[colourSlot] = gPlttBufferFaded[colourSlot];
            gPlttBufferUnfaded[colourSlot] = gLightingColours[i].lightColour;
        }
        else
        {
            sDnsPaletteDmaBuffer[colourSlot] = gLightingColours[i].lightColour;
        }
    }
}

// Checks if current map is affected by dns
static bool8 IsMapDNSException()
{
    u8 i;
    for (i = 0; i < sizeof(gDnsMapExceptions)/sizeof(gDnsMapExceptions[0]); i++)
    {
        if (gMapHeader.mapType == gDnsMapExceptions[i])
            return TRUE;
    }
    return FALSE;
}

// Returns true if the palette should not be affected by DNS filtering
static bool8 IsSpritePaletteTagDnsException(u8 palNum)
{
    u8 i;

    for (i = 0; i < sizeof(gPaletteTagExceptions)/sizeof(gPaletteTagExceptions[0]); i++)
    {
        if (GetSpritePaletteTagByPaletteNum(palNum) == gPaletteTagExceptions[i])
            return TRUE;
    }
    return FALSE;
}

// Returns true if overworld is running
static bool8 IsOverworld()
{
    return (gMain.callback2 == CB2_Overworld || gMain.callback2 == CB2_OverworldBasic);
}

// Returns true if combat is running
static bool8 IsCombat()
{
    return (gMain.callback2 == BattleMainCB2);
}

// Checks if lightning effect should be active
static bool8 IsLightActive()
{
    return (gLocalTime.hours >= LIGHTNING_START_HOUR || gLocalTime.hours < LIGHTNING_END_HOUR);
}

// Add the definition of GetDNSFilter() if it is relevant.
